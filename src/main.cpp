#include "sdlException.hpp"
#include "shaderUtils.hpp"
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_stdinc.h>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

int main() {
  atexit(SDL_Quit);
  if (!SDL_Init(SDL_INIT_VIDEO))
    throw SDL_Exception("SDL_Init failed!");

  SDL_Window *window =
      SDL_CreateWindow("Hello World", 800, 600, SDL_WINDOW_RESIZABLE);
  if (!window)
    throw SDL_Exception("SDL_CreateWindow failed!");

  SDL_GPUDevice *device =
      SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, nullptr);
  if (!device)
    throw SDL_Exception("SDL_CreateGPUDevice failed!");

  if (!SDL_ClaimWindowForGPUDevice(device, window))
    throw SDL_Exception("SDL_ClaimWindowForGPUDevice failed!");

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

  SDL_GPUVertexAttribute vertexAttribute{};
  vertexAttribute.buffer_slot = 0;
  vertexAttribute.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
  vertexAttribute.location = 0;
  vertexAttribute.offset = 0;

  SDL_GPUVertexBufferDescription vertexBufferDescription{};
  vertexBufferDescription.slot = 0;
  vertexBufferDescription.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
  vertexBufferDescription.instance_step_rate = 0;
  vertexBufferDescription.pitch = sizeof(glm::vec3);

  SDL_GPUGraphicsPipelineCreateInfo createInfo{};
  createInfo.vertex_shader = vertexShader;
  createInfo.vertex_input_state.num_vertex_buffers = 1;
  createInfo.vertex_input_state.vertex_buffer_descriptions =
      &vertexBufferDescription;
  createInfo.vertex_input_state.num_vertex_attributes = 1;
  createInfo.vertex_input_state.vertex_attributes = &vertexAttribute;
  createInfo.fragment_shader = fragmentShader;
  createInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
  createInfo.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
  createInfo.target_info = targetInfo;
  createInfo.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_NONE;

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
                                          1.0f, // aspect ratio
                                          0.1f, 100.0f);
  glm::mat4 mvp = projection * view;

  std::vector<glm::vec3> vertices = {
      {-0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, -0.5f},
      {-0.5f, 0.5f, -0.5f},  {-0.5f, -0.5f, 0.5f}, {0.5f, -0.5f, 0.5f},
      {0.5f, 0.5f, 0.5f},    {-0.5f, 0.5f, 0.5f},
  };

  std::vector<Uint32> indices = {// back
                                 0, 1, 2, 0, 2, 3,

                                 // front
                                 4, 6, 5, 4, 7, 6,

                                 // top
                                 3, 2, 6, 3, 6, 7,

                                 // bottom
                                 0, 5, 1, 0, 4, 5,

                                 // left
                                 0, 3, 7, 0, 7, 4,

                                 // right
                                 1, 5, 6, 1, 6, 2};
  SDL_GPUBufferCreateInfo vertexBufferCreateInfo{};
  vertexBufferCreateInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
  vertexBufferCreateInfo.size = sizeof(glm::vec3) * vertices.size();
  SDL_GPUBuffer *vertexBuffer =
      SDL_CreateGPUBuffer(device, &vertexBufferCreateInfo);
  if (!vertexBuffer)
    throw SDL_Exception("Failed to create vertexBuffer!");

  SDL_GPUBufferCreateInfo indexBufferCreateInfo{};
  indexBufferCreateInfo.usage = SDL_GPU_BUFFERUSAGE_INDEX;
  indexBufferCreateInfo.size = sizeof(Uint32) * indices.size();
  SDL_GPUBuffer *indexBuffer =
      SDL_CreateGPUBuffer(device, &indexBufferCreateInfo);
  if (!indexBuffer)
    throw SDL_Exception("Failed to create indexBuffer!");

  SDL_GPUTransferBufferCreateInfo vertexTransferBufferCreateInfo{};
  vertexTransferBufferCreateInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
  vertexTransferBufferCreateInfo.size = sizeof(glm::vec3) * vertices.size();

  SDL_GPUTransferBufferCreateInfo indexTransferBufferCreateInfo{};
  indexTransferBufferCreateInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
  indexTransferBufferCreateInfo.size = sizeof(Uint32) * indices.size();

  SDL_GPUTransferBuffer *vertexTransferBuffer =
      SDL_CreateGPUTransferBuffer(device, &vertexTransferBufferCreateInfo);
  if (!vertexTransferBuffer)
    throw SDL_Exception("Failed to create vertexTransferBuffer");

  SDL_GPUTransferBuffer *indexTransferBuffer =
      SDL_CreateGPUTransferBuffer(device, &indexTransferBufferCreateInfo);
  if (!indexTransferBuffer)
    throw SDL_Exception("Failed to create indexTransferBuffer");

  glm::vec3 *vertexTransferBufferPointer = static_cast<glm::vec3 *>(
      SDL_MapGPUTransferBuffer(device, vertexTransferBuffer, true));
  if (!vertexTransferBufferPointer)
    throw SDL_Exception("Failed to create vertexTransferBufferPointer");

  Uint32 *indexTransferBufferPointer = static_cast<Uint32 *>(
      SDL_MapGPUTransferBuffer(device, indexTransferBuffer, true));
  if (!indexTransferBufferPointer)
    throw SDL_Exception("Failed to create indexTransferBufferPointer");

  SDL_memcpy(vertexTransferBufferPointer, vertices.data(),
             sizeof(glm::vec3) * vertices.size());

  SDL_memcpy(indexTransferBufferPointer, indices.data(),
             sizeof(Uint32) * indices.size());

  SDL_UnmapGPUTransferBuffer(device, indexTransferBuffer);
  SDL_UnmapGPUTransferBuffer(device, vertexTransferBuffer);

  SDL_GPUCommandBuffer *uploadCommandBuffer =
      SDL_AcquireGPUCommandBuffer(device);
  if (!uploadCommandBuffer)
    throw SDL_Exception("Failed to acquire command Buffer");
  SDL_GPUCopyPass *copypass = SDL_BeginGPUCopyPass(uploadCommandBuffer);

  SDL_GPUTransferBufferLocation indexTransferBufferSource{};
  indexTransferBufferSource.transfer_buffer = indexTransferBuffer;
  indexTransferBufferSource.offset = 0;
  SDL_GPUBufferRegion indexTransferBufferDest{};
  indexTransferBufferDest.buffer = indexBuffer;
  indexTransferBufferDest.offset = 0;
  indexTransferBufferDest.size = indexBufferCreateInfo.size;
  SDL_UploadToGPUBuffer(copypass, &indexTransferBufferSource,
                        &indexTransferBufferDest, true);

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
  SDL_ReleaseGPUTransferBuffer(device, indexTransferBuffer);

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
      SDL_GPURenderPass *renderPass = SDL_BeginGPURenderPass(
          commandBuffer, colorTargets.data(), colorTargets.size(), nullptr);
      SDL_BindGPUGraphicsPipeline(renderPass, pipeline);
      SDL_PushGPUVertexUniformData(commandBuffer, 0, &mvp, sizeof(mvp));
      SDL_GPUBufferBinding vertexBufferBinding{};
      vertexBufferBinding.buffer = vertexBuffer;
      vertexBufferBinding.offset = 0;
      SDL_BindGPUVertexBuffers(renderPass, 0, &vertexBufferBinding, 1);
      SDL_GPUBufferBinding indexBufferBinding{};
      indexBufferBinding.buffer = indexBuffer;
      indexBufferBinding.offset = 0;
      SDL_BindGPUIndexBuffer(renderPass, &indexBufferBinding,
                             SDL_GPU_INDEXELEMENTSIZE_32BIT);
      SDL_DrawGPUIndexedPrimitives(renderPass, indices.size(), 1, 0, 0, 0);
      SDL_EndGPURenderPass(renderPass);
    }
    if (!SDL_SubmitGPUCommandBuffer(commandBuffer))
      throw SDL_Exception("SDL_SubmitGPUCommandBuffer failed!");
  }
  return EXIT_SUCCESS;
}
