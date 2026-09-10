#include "sdlException.hpp"
#include "shaderUtils.hpp"
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_stdinc.h>
#include <glm/ext/vector_float3.hpp>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <renderer.hpp>

int main() {
  atexit(SDL_Quit);
  Renderer renderer = Renderer();

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
    renderer.Render();
  }
  return EXIT_SUCCESS;
}
