#include "vertex.hpp"
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gpu.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <vector>

#define WIDTH 640.0f  // 320.0f
#define HEIGHT 360.0f // 180.0f

class Renderer {
public:
  Renderer();
  ~Renderer();
  void Render();

private:
  SDL_Window *window;
  SDL_GPUDevice *device = nullptr;
  SDL_GPUShader *vertexShader;
  SDL_GPUShader *fragmentShader;
  std::vector<SDL_GPUColorTargetDescription> colorTargetDescriptions{};
  SDL_GPUGraphicsPipelineTargetInfo targetInfo{};
  std::vector<SDL_GPUVertexAttribute> vertexAttributes{};
  SDL_GPUVertexBufferDescription vertexBufferDescription{};
  SDL_GPUGraphicsPipelineCreateInfo createInfo{};

  glm::mat4 model = glm::mat4(1.0f);
  glm::mat4 view = glm::lookAt(glm::vec3(3.0f, 3.0f, 3.0f), // eye
                               glm::vec3(0.0f, 0.0f, 0.0f), // target
                               glm::vec3(0.0f, 1.0f, 0.0f)  // up
  );

  glm::mat4 projection = glm::perspective(glm::radians(45.0f),
                                          WIDTH / HEIGHT, // aspect ratio
                                          0.1f, 100.0f);

  SDL_GPUGraphicsPipeline *pipeline;
  SDL_GPUBufferCreateInfo vertexBufferCreateInfo{};
  SDL_GPUBuffer *vertexBuffer;
  SDL_GPUTransferBufferCreateInfo vertexTransferBufferCreateInfo{};
  SDL_GPUTransferBuffer *vertexTransferBuffer;
  Vertex *vertexTransferBufferPointer;
  SDL_GPUTransferBufferLocation vertexTransferBufferSource{};
  SDL_GPUBufferRegion vertexTransferBufferDest{};
  SDL_GPUTextureCreateInfo depthStencilCreateInfo{};
  SDL_GPUTexture *depthStencilTexture;
  SDL_GPUDepthStencilTargetInfo depthStencilTargetInfo{};

  SDL_GPUCommandBuffer *commandBuffer = SDL_AcquireGPUCommandBuffer(device);
  std::vector<SDL_GPUColorTargetInfo> colorTargets{};

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
};
