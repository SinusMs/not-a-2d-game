#include <iostream>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_gpu.h>
#include <vector>
#include <format>
#include <SDL3/SDL_filesystem.h>
#include <fstream>


class SDL_Exception final : public std::runtime_error {
public:
	explicit SDL_Exception(const std::string &message) : std::runtime_error(message + '\n' + SDL_GetError()) {
	}
};

static std::string BasePath{};

void InitializeAssetLoader() {
	BasePath = std::string{SDL_GetBasePath()};
}

SDL_GPUShader *LoadShader(
	SDL_GPUDevice *device,
	const std::string &shaderFilename,
	const Uint32 samplerCount,
	const Uint32 uniformBufferCount,
	const Uint32 storageBufferCount,
	const Uint32 storageTextureCount
) {
	// Auto-detect the shader stage from the file name for convenience
	SDL_GPUShaderStage stage;
	if (shaderFilename.find(".vert") != std::string::npos)
		stage = SDL_GPU_SHADERSTAGE_VERTEX;
	else if (shaderFilename.find(".frag")!= std::string::npos)
		stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
	else
		throw std::runtime_error{"Unrecognized shader stage!"};

	std::string fullPath;
	const SDL_GPUShaderFormat backendFormats = SDL_GetGPUShaderFormats(device);
	SDL_GPUShaderFormat format = SDL_GPU_SHADERFORMAT_INVALID;
	const char *entrypoint;

	if (backendFormats & SDL_GPU_SHADERFORMAT_SPIRV) {
		fullPath = std::format("{}/Content/Shaders/Compiled/SPIRV/{}.spv", BasePath, shaderFilename);
		format = SDL_GPU_SHADERFORMAT_SPIRV;
		entrypoint = "main";
	} else if (backendFormats & SDL_GPU_SHADERFORMAT_MSL) {
		fullPath = std::format("{}/Content/Shaders/Compiled/MSL/{}.msl", BasePath, shaderFilename);
		format = SDL_GPU_SHADERFORMAT_MSL;
		entrypoint = "main0";
	} else if (backendFormats & SDL_GPU_SHADERFORMAT_DXIL) {
		fullPath = std::format("{}/Content/Shaders/Compiled/DXIL/{}.dxil", BasePath, shaderFilename);
		format = SDL_GPU_SHADERFORMAT_DXIL;
		entrypoint = "main";
	} else throw std::runtime_error{"No supported shader formats available"};

	std::ifstream file{fullPath, std::ios::binary};
	if (!file)
		throw std::runtime_error{"Couldn't open shader file"};
	std::vector<Uint8> code{std::istreambuf_iterator(file), {}};

	SDL_GPUShaderCreateInfo shaderInfo{};
	shaderInfo.code = code.data();
	shaderInfo.code_size = code.size();
	shaderInfo.entrypoint = entrypoint;
	shaderInfo.format = format;
	shaderInfo.stage = stage;
	shaderInfo.num_samplers = samplerCount;
	shaderInfo.num_uniform_buffers = uniformBufferCount;
	shaderInfo.num_storage_buffers = storageBufferCount;
	shaderInfo.num_storage_textures = storageTextureCount;

	SDL_GPUShader *shader = SDL_CreateGPUShader(device, &shaderInfo);
	if (!shader)
		throw SDL_Exception{"Couldn't create GPU shader"};

	return shader;
}

int main(){
    if(!SDL_Init(SDL_INIT_VIDEO))
        throw SDL_Exception("SDL_Init failed!");

    InitializeAssetLoader();
    
    SDL_Window* window = SDL_CreateWindow("Hello World", 800, 600, SDL_WINDOW_RESIZABLE);
    if(!window)
        throw SDL_Exception("SDL_CreateWindow failed!");

    SDL_GPUDevice* device =  SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_DXIL, true, nullptr);
    if(!device)
        throw SDL_Exception("SDL_CreateGPUDevice failed!");

    if(!SDL_ClaimWindowForGPUDevice(device, window))
        throw SDL_Exception("SDL_ClaimWindowForGPUDevice failed!");

    SDL_GPUShader* vertexShader = LoadShader(device, "RawTriangle.vert", 0, 0, 0, 0);
    if(!vertexShader)
        throw SDL_Exception("LoadShader failed!");

    SDL_GPUShader* fragmentShader = LoadShader(device, "SolidColor.frag", 0, 0, 0, 0);
    if(!fragmentShader)
        throw SDL_Exception("LoadShader failed!");
    

    SDL_GPUColorTargetDescription colorTargetDescription{};
    colorTargetDescription.format = SDL_GetGPUSwapchainTextureFormat(device, window);
    std::vector colorTargetDescriptions{ colorTargetDescription };

    SDL_GPUGraphicsPipelineTargetInfo targetInfo{};
    targetInfo.color_target_descriptions = colorTargetDescriptions.data();
    targetInfo.num_color_targets = colorTargetDescriptions.size();

    SDL_GPUGraphicsPipelineCreateInfo createInfo{};
    createInfo.vertex_shader = vertexShader;
    createInfo.fragment_shader = fragmentShader;
    createInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    createInfo.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
    createInfo.target_info = targetInfo;

    SDL_GPUGraphicsPipeline* pipeline = SDL_CreateGPUGraphicsPipeline(device, &createInfo);
    if(!pipeline)
        throw SDL_Exception("SDL_CreateGPUGraphicsPipeline failed!");

    SDL_ReleaseGPUShader(device, vertexShader);
    SDL_ReleaseGPUShader(device, fragmentShader);
    
    SDL_ShowWindow(window);

    bool running = true;
    SDL_Event event;
    while(running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) 
            {
                case SDL_EVENT_QUIT:
                    running = false;
                    break;
                default:
                    break;
            }
        }

        SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(device);
        if (!commandBuffer)
            throw SDL_Exception("SDL_AcquireGPUCommandBuffer failed!");
        
        SDL_GPUTexture* swapchainTexture;
        if (!SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer, window, &swapchainTexture, nullptr, nullptr))
            throw SDL_Exception("SDL_WaitAndAcquireGPUSwapchainTexture failed!");

        if (swapchainTexture) {
            SDL_GPUColorTargetInfo colorTargetInfo{}; //???
            colorTargetInfo.texture = swapchainTexture;
            colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
            colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
            colorTargetInfo.clear_color = {0.2f, 0.2f, 0.2f, 1.0f};
            std::vector colorTargets{ colorTargetInfo };
            SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(commandBuffer, colorTargets.data(), colorTargets.size(), nullptr);
            SDL_BindGPUGraphicsPipeline(renderPass, pipeline);
            SDL_DrawGPUPrimitives(renderPass, 3, 1, 0, 0); // Draw a triangle with 3 vertices
            SDL_EndGPURenderPass(renderPass);
        }
        if (!SDL_SubmitGPUCommandBuffer(commandBuffer))
            throw SDL_Exception("SDL_SubmitGPUCommandBuffer failed!");
    }

    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    return EXIT_SUCCESS;
}
