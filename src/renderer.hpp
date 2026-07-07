#include <SDL3/SDL_gpu.h>
#include <string>

class Renderer {
    public:
        Renderer();
        ~Renderer();
    private:
        SDL_GPUShader *LoadShader(
            SDL_GPUDevice *device,
            const std::string &shaderFilename,
            const Uint32 samplerCount,
            const Uint32 uniformBufferCount,
            const Uint32 storageBufferCount,
            const Uint32 storageTextureCount
        );

};