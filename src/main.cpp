#include "sdlException.hpp"
#include "shaderUtils.hpp"
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_stdinc.h>
#include <cstddef>
#include <glm/ext/vector_float3.hpp>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

int main() {
  atexit(SDL_Quit);
  if (!SDL_Init(SDL_INIT_VIDEO))
    throw SDL_Exception("SDL_Init failed!");

  SDL_Window *window = SDL_CreateWindow("Hello World", 800, 600, 0);
  if (!window)
    throw SDL_Exception("SDL_CreateWindow failed!");

  SDL_GPUDevice *device =
      SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, nullptr);
  if (!device)
    throw SDL_Exception("SDL_CreateGPUDevice failed!");

  if (!SDL_ClaimWindowForGPUDevice(device, window))
    throw SDL_Exception("SDL_ClaimWindowForGPUDevice failed!");

  struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
  };

  SDL_GPUShader *vertexShader =
      ShaderUtils::LoadShader(device, "cube.vert", 0, 1, 0, 0);
  if (!vertexShader)
    throw SDL_Exception("LoadShader failed!");

  SDL_GPUShader *fragmentShader =
      ShaderUtils::LoadShader(device, "cube.frag", 0, 0, 0, 0);
  if (!fragmentShader)
    throw SDL_Exception("LoadShader failed!");

  SDL_GPUColorTargetDescription colorTargetDescription{};
  colorTargetDescription.format =
      SDL_GetGPUSwapchainTextureFormat(device, window);
  std::vector colorTargetDescriptions{colorTargetDescription};

  SDL_GPUGraphicsPipelineTargetInfo targetInfo{};
  targetInfo.color_target_descriptions = colorTargetDescriptions.data();
  targetInfo.num_color_targets = colorTargetDescriptions.size();
  targetInfo.depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D16_UNORM;
  targetInfo.has_depth_stencil_target = true;

  SDL_GPUVertexAttribute vertexAttributes[] = {
      {
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

  SDL_GPUVertexBufferDescription vertexBufferDescription{};
  vertexBufferDescription.slot = 0;
  vertexBufferDescription.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
  vertexBufferDescription.instance_step_rate = 0;
  vertexBufferDescription.pitch = sizeof(Vertex);

  SDL_GPUGraphicsPipelineCreateInfo createInfo{};
  createInfo.vertex_shader = vertexShader;
  createInfo.vertex_input_state.num_vertex_buffers = 1;
  createInfo.vertex_input_state.vertex_buffer_descriptions =
      &vertexBufferDescription;
  createInfo.vertex_input_state.num_vertex_attributes = 2;
  createInfo.vertex_input_state.vertex_attributes = vertexAttributes;
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

  SDL_GPUGraphicsPipeline *pipeline =
      SDL_CreateGPUGraphicsPipeline(device, &createInfo);
  if (!pipeline)
    throw SDL_Exception("SDL_CreateGPUGraphicsPipeline failed!");

  SDL_ReleaseGPUShader(device, vertexShader);
  SDL_ReleaseGPUShader(device, fragmentShader);

  glm::mat4 view = glm::lookAt(glm::vec3(3.0f, 3.0f, 3.0f), // eye
                               glm::vec3(0.0f, 0.0f, 0.0f), // target
                               glm::vec3(0.0f, 1.0f, 0.0f)  // up
  );

  glm::mat4 projection = glm::perspective(glm::radians(45.0f),
                                          4.0f / 3.0f, // aspect ratio
                                          0.1f, 100.0f);
  glm::mat4 mvp = projection * view;

  const glm::vec3 up = {0.0f, 1.0f, 0.0f};
  const glm::vec3 down = {0.0f, -1.0f, 0.0f};
  const glm::vec3 right = {1.0f, 0.0f, 0.0f};
  const glm::vec3 left = {-1.0f, 0.0f, 0.0f};
  const glm::vec3 front = {0.0f, 0.0f, -1.0f};
  const glm::vec3 back = {0.0f, 0.0f, 1.0f};

  std::vector<Vertex> verts = {
      // Front (-Z)
      {{-0.5f, -0.5f, -0.5f}, front},
      {{0.5f, -0.5f, -0.5f}, front},
      {{0.5f, 0.5f, -0.5f}, front},

      {{-0.5f, -0.5f, -0.5f}, front},
      {{0.5f, 0.5f, -0.5f}, front},
      {{-0.5f, 0.5f, -0.5f}, front},

      // Back (+Z)
      {{0.5f, -0.5f, 0.5f}, back},
      {{-0.5f, -0.5f, 0.5f}, back},
      {{-0.5f, 0.5f, 0.5f}, back},

      {{0.5f, -0.5f, 0.5f}, back},
      {{-0.5f, 0.5f, 0.5f}, back},
      {{0.5f, 0.5f, 0.5f}, back},

      // Left (-X)
      {{-0.5f, -0.5f, 0.5f}, left},
      {{-0.5f, -0.5f, -0.5f}, left},
      {{-0.5f, 0.5f, -0.5f}, left},

      {{-0.5f, -0.5f, 0.5f}, left},
      {{-0.5f, 0.5f, -0.5f}, left},
      {{-0.5f, 0.5f, 0.5f}, left},

      // Right (+X)
      {{0.5f, -0.5f, -0.5f}, right},
      {{0.5f, -0.5f, 0.5f}, right},
      {{0.5f, 0.5f, 0.5f}, right},

      {{0.5f, -0.5f, -0.5f}, right},
      {{0.5f, 0.5f, 0.5f}, right},
      {{0.5f, 0.5f, -0.5f}, right},

      // Top (+Y)
      {{-0.5f, 0.5f, -0.5f}, up},
      {{0.5f, 0.5f, -0.5f}, up},
      {{0.5f, 0.5f, 0.5f}, up},

      {{-0.5f, 0.5f, -0.5f}, up},
      {{0.5f, 0.5f, 0.5f}, up},
      {{-0.5f, 0.5f, 0.5f}, up},

      // Bottom (-Y)
      {{-0.5f, -0.5f, 0.5f}, down},
      {{0.5f, -0.5f, 0.5f}, down},
      {{0.5f, -0.5f, -0.5f}, down},

      {{-0.5f, -0.5f, 0.5f}, down},
      {{0.5f, -0.5f, -0.5f}, down},
      {{-0.5f, -0.5f, -0.5f}, down},
  };

  SDL_GPUBufferCreateInfo vertexBufferCreateInfo{};
  vertexBufferCreateInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
  vertexBufferCreateInfo.size = sizeof(Vertex) * verts.size();
  SDL_GPUBuffer *vertexBuffer =
      SDL_CreateGPUBuffer(device, &vertexBufferCreateInfo);
  if (!vertexBuffer)
    throw SDL_Exception("Failed to create vertexBuffer!");

  SDL_GPUTransferBufferCreateInfo vertexTransferBufferCreateInfo{};
  vertexTransferBufferCreateInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
  vertexTransferBufferCreateInfo.size = sizeof(Vertex) * verts.size();

  SDL_GPUTransferBuffer *vertexTransferBuffer =
      SDL_CreateGPUTransferBuffer(device, &vertexTransferBufferCreateInfo);
  if (!vertexTransferBuffer)
    throw SDL_Exception("Failed to create vertexTransferBuffer");

  Vertex *vertexTransferBufferPointer = static_cast<Vertex *>(
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

  SDL_GPUTransferBufferLocation vertexTransferBufferSource{};
  vertexTransferBufferSource.transfer_buffer = vertexTransferBuffer;
  vertexTransferBufferSource.offset = 0;
  SDL_GPUBufferRegion vertexTransferBufferDest{};
  vertexTransferBufferDest.buffer = vertexBuffer;
  vertexTransferBufferDest.offset = 0;
  vertexTransferBufferDest.size = vertexBufferCreateInfo.size;
  SDL_UploadToGPUBuffer(copypass, &vertexTransferBufferSource,
                        &vertexTransferBufferDest, true);

  SDL_EndGPUCopyPass(copypass);
  if (!SDL_SubmitGPUCommandBuffer(uploadCommandBuffer))
    throw SDL_Exception("Failed to submit CommandBuffer");
  SDL_ReleaseGPUTransferBuffer(device, vertexTransferBuffer);

  SDL_GPUTextureCreateInfo depthStencilCreateInfo{};
  depthStencilCreateInfo.type = SDL_GPU_TEXTURETYPE_2D;
  depthStencilCreateInfo.format = SDL_GPU_TEXTUREFORMAT_D16_UNORM;
  depthStencilCreateInfo.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
  depthStencilCreateInfo.width = 800;
  depthStencilCreateInfo.height = 600;
  depthStencilCreateInfo.layer_count_or_depth = 1;
  depthStencilCreateInfo.num_levels = 1;

  SDL_GPUTexture *depthStencilTexture =
      SDL_CreateGPUTexture(device, &depthStencilCreateInfo);
  if (!depthStencilTexture) {
    throw SDL_Exception("Creating depthStencilTexture failed!");
  }

  SDL_GPUDepthStencilTargetInfo depthStencilTargetInfo{};
  depthStencilTargetInfo.texture = depthStencilTexture;
  depthStencilTargetInfo.clear_depth = 1;
  depthStencilTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
  depthStencilTargetInfo.store_op = SDL_GPU_STOREOP_DONT_CARE;
  depthStencilTargetInfo.cycle = false;

  SDL_ShowWindow(window);

  bool running = true;
  SDL_Event event;
  while (running) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_EVENT_QUIT:
        running = false;
        break;
      default:
        break;
      }
    }

    SDL_GPUCommandBuffer *commandBuffer = SDL_AcquireGPUCommandBuffer(device);
    if (!commandBuffer)
      throw SDL_Exception("SDL_AcquireGPUCommandBuffer failed!");

    SDL_GPUTexture *swapchainTexture;
    if (!SDL_WaitAndAcquireGPUSwapchainTexture(
            commandBuffer, window, &swapchainTexture, nullptr, nullptr))
      throw SDL_Exception("SDL_WaitAndAcquireGPUSwapchainTexture failed!");

    if (swapchainTexture) {
      SDL_GPUColorTargetInfo colorTargetInfo{}; //???
      colorTargetInfo.texture = swapchainTexture;
      colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
      colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
      colorTargetInfo.clear_color = {0.2f, 0.2f, 0.2f, 1.0f};
      std::vector colorTargets{colorTargetInfo};
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
  return EXIT_SUCCESS;
}
