#include "shaderUtils.hpp"
#include "sdlException.hpp"
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_gpu.h>
#include <format>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

SDL_GPUShader *ShaderUtils::LoadShader(SDL_GPUDevice *device,
                                       const std::string &shaderFilename,
                                       const Uint32 samplerCount,
                                       const Uint32 uniformBufferCount,
                                       const Uint32 storageBufferCount,
                                       const Uint32 storageTextureCount) {
  std::string BasePath = SDL_GetBasePath();
  // Auto-detect the shader stage from the file name for convenience
  SDL_GPUShaderStage stage;
  if (shaderFilename.find(".vert") != std::string::npos)
    stage = SDL_GPU_SHADERSTAGE_VERTEX;
  else if (shaderFilename.find(".frag") != std::string::npos)
    stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
  else
    throw std::runtime_error{"Unrecognized shader stage!"};

  std::string fullPath;
  const SDL_GPUShaderFormat backendFormats = SDL_GetGPUShaderFormats(device);
  SDL_GPUShaderFormat format = SDL_GPU_SHADERFORMAT_INVALID;
  const char *entrypoint;

  if (backendFormats & SDL_GPU_SHADERFORMAT_SPIRV) {
    fullPath = std::format("{}shaders/spv/{}.spv", BasePath, shaderFilename);
    format = SDL_GPU_SHADERFORMAT_SPIRV;
    entrypoint = "main";
  } else
    throw std::runtime_error{"No supported shader formats available"};

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
