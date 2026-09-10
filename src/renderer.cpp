#include "sdlException.hpp"
#include "shaderUtils.hpp"
#include <SDL3/SDL_init.h>
#include <renderer.hpp>

Renderer::Renderer() {
  if (!SDL_InitSubSystem(SDL_INIT_VIDEO))
    throw SDL_Exception("SDL_Init Video failed!");

  window = SDL_CreateWindow("Hello World", WIDTH, HEIGHT, 0);
  if (!window)
    throw SDL_Exception("SDL_CreateWindow failed!");

  device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, nullptr);
  if (!device)
    throw SDL_Exception("SDL_CreateGPUDevice failed!");

  if (!SDL_ClaimWindowForGPUDevice(device, window))
    throw SDL_Exception("SDL_ClaimWindowForGPUDevice failed!");

  vertexShader = ShaderUtils::LoadShader(device, "cube.vert", 0, 1, 0, 0);
  if (!vertexShader)
    throw SDL_Exception("LoadShader failed!");

  fragmentShader = ShaderUtils::LoadShader(device, "cube.frag", 0, 0, 0, 0);
  if (!fragmentShader)
    throw SDL_Exception("LoadShader failed!");

  SDL_GPUColorTargetDescription colorTargetDescription{};
  colorTargetDescription.format =
      SDL_GetGPUSwapchainTextureFormat(device, window);
  colorTargetDescriptions = {colorTargetDescription};

  targetInfo.color_target_descriptions = colorTargetDescriptions.data();
  targetInfo.num_color_targets = colorTargetDescriptions.size();
  targetInfo.depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D16_UNORM;
  targetInfo.has_depth_stencil_target = true;

  vertexAttributes = {{
                          .location = 0,
                          .buffer_slot = 0,
                          .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
                          .offset = offsetof(Vertex, pos),
                      },
                      {
                          .location = 1,
                          .buffer_slot = 0,
                          .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
                          .offset = offsetof(Vertex, normal),
                      }};

  vertexBufferDescription.slot = 0;
  vertexBufferDescription.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
  vertexBufferDescription.instance_step_rate = 0;
  vertexBufferDescription.pitch = sizeof(Vertex);

  createInfo.vertex_shader = vertexShader;
  createInfo.vertex_input_state.num_vertex_buffers = 1;
  createInfo.vertex_input_state.vertex_buffer_descriptions =
      &vertexBufferDescription;
  createInfo.vertex_input_state.num_vertex_attributes = 2;
  createInfo.vertex_input_state.vertex_attributes = vertexAttributes.data();
  createInfo.fragment_shader = fragmentShader;
  createInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
  createInfo.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
  createInfo.target_info = targetInfo;
  createInfo.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_BACK;
  createInfo.rasterizer_state.enable_depth_clip = false;
  createInfo.rasterizer_state.front_face = SDL_GPU_FRONTFACE_CLOCKWISE;
  createInfo.depth_stencil_state.enable_depth_test = true;
  createInfo.depth_stencil_state.enable_depth_write = true;
  createInfo.depth_stencil_state.compare_op = SDL_GPU_COMPAREOP_LESS;

  pipeline = SDL_CreateGPUGraphicsPipeline(device, &createInfo);
  if (!pipeline)
    throw SDL_Exception("SDL_CreateGPUGraphicsPipeline failed!");

  SDL_ReleaseGPUShader(device, vertexShader);
  SDL_ReleaseGPUShader(device, fragmentShader);

  vertexBufferCreateInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
  vertexBufferCreateInfo.size = sizeof(Vertex) * verts.size();
  vertexBuffer = SDL_CreateGPUBuffer(device, &vertexBufferCreateInfo);
  if (!vertexBuffer)
    throw SDL_Exception("Failed to create vertexBuffer!");

  vertexTransferBufferCreateInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
  vertexTransferBufferCreateInfo.size = sizeof(Vertex) * verts.size();

  vertexTransferBuffer =
      SDL_CreateGPUTransferBuffer(device, &vertexTransferBufferCreateInfo);
  if (!vertexTransferBuffer)
    throw SDL_Exception("Failed to create vertexTransferBuffer");

  vertexTransferBufferPointer = static_cast<Vertex *>(
      SDL_MapGPUTransferBuffer(device, vertexTransferBuffer, true));
  if (!vertexTransferBufferPointer)
    throw SDL_Exception("Failed to create vertexTransferBufferPointer");

  SDL_memcpy(vertexTransferBufferPointer, verts.data(),
             sizeof(Vertex) * verts.size());

  SDL_UnmapGPUTransferBuffer(device, vertexTransferBuffer);

  SDL_GPUCommandBuffer *uploadCommandBuffer =
      SDL_AcquireGPUCommandBuffer(device);
  if (!uploadCommandBuffer)
    throw SDL_Exception("Failed to acquire command Buffer");
  SDL_GPUCopyPass *copypass = SDL_BeginGPUCopyPass(uploadCommandBuffer);

  vertexTransferBufferSource.transfer_buffer = vertexTransferBuffer;
  vertexTransferBufferSource.offset = 0;
  vertexTransferBufferDest.buffer = vertexBuffer;
  vertexTransferBufferDest.offset = 0;
  vertexTransferBufferDest.size = vertexBufferCreateInfo.size;
  SDL_UploadToGPUBuffer(copypass, &vertexTransferBufferSource,
                        &vertexTransferBufferDest, true);

  SDL_EndGPUCopyPass(copypass);
  if (!SDL_SubmitGPUCommandBuffer(uploadCommandBuffer))
    throw SDL_Exception("Failed to submit CommandBuffer");
  SDL_ReleaseGPUTransferBuffer(device, vertexTransferBuffer);

  depthStencilCreateInfo.type = SDL_GPU_TEXTURETYPE_2D;
  depthStencilCreateInfo.format = SDL_GPU_TEXTUREFORMAT_D16_UNORM;
  depthStencilCreateInfo.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
  depthStencilCreateInfo.width = WIDTH;
  depthStencilCreateInfo.height = HEIGHT;
  depthStencilCreateInfo.layer_count_or_depth = 1;
  depthStencilCreateInfo.num_levels = 1;

  depthStencilTexture = SDL_CreateGPUTexture(device, &depthStencilCreateInfo);
  if (!depthStencilTexture) {
    throw SDL_Exception("Creating depthStencilTexture failed!");
  }

  depthStencilTargetInfo.texture = depthStencilTexture;
  depthStencilTargetInfo.clear_depth = 1;
  depthStencilTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
  depthStencilTargetInfo.store_op = SDL_GPU_STOREOP_DONT_CARE;
  depthStencilTargetInfo.cycle = false;

  SDL_ShowWindow(window);
}

Renderer::~Renderer() { SDL_QuitSubSystem(SDL_INIT_VIDEO); }

void Renderer::Render() {
  glm::mat4 mvp = projection * view * model;
  commandBuffer = SDL_AcquireGPUCommandBuffer(device);
  if (!commandBuffer)
    throw SDL_Exception("SDL_AcquireGPUCommandBuffer failed!");

  SDL_GPUTexture *swapchainTexture;
  if (!SDL_WaitAndAcquireGPUSwapchainTexture(
          commandBuffer, window, &swapchainTexture, nullptr, nullptr))
    throw SDL_Exception("SDL_WaitAndAcquireGPUSwapchainTexture failed!");

  if (swapchainTexture) {
    SDL_GPUColorTargetInfo colorTargetInfo{};
    colorTargetInfo.texture = swapchainTexture;
    colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
    colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
    colorTargetInfo.clear_color = {0.2f, 0.2f, 0.2f, 1.0f};
    colorTargets = {colorTargetInfo};
    SDL_GPURenderPass *renderPass =
        SDL_BeginGPURenderPass(commandBuffer, colorTargets.data(),
                               colorTargets.size(), &depthStencilTargetInfo);
    SDL_BindGPUGraphicsPipeline(renderPass, pipeline);
    SDL_PushGPUVertexUniformData(commandBuffer, 0, &mvp, sizeof(mvp));
    SDL_GPUBufferBinding vertexBufferBinding{};
    vertexBufferBinding.buffer = vertexBuffer;
    vertexBufferBinding.offset = 0;
    SDL_BindGPUVertexBuffers(renderPass, 0, &vertexBufferBinding, 1);
    SDL_DrawGPUPrimitives(renderPass, verts.size(), 1, 0, 0);
    SDL_EndGPURenderPass(renderPass);
  }
  if (!SDL_SubmitGPUCommandBuffer(commandBuffer))
    throw SDL_Exception("SDL_SubmitGPUCommandBuffer failed!");
}
