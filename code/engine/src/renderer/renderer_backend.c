#include "renderer_backend.h"

#include "vulkan/vulkan_backend.h"


b8 RendererBackendCreate(renderer_backend_type Type, renderer_backend* OutRendererBackend)
{
    if(Type == RENDERER_BACKEND_TYPE_VULKAN)
    {
        OutRendererBackend->Initialize        = VulkanRendererBackendInitialize;
        OutRendererBackend->Shutdown          = VulkanRendererBackendShutdown;
        OutRendererBackend->BeginFrame        = VulkanRendererBackendBeginFrame;
        OutRendererBackend->UpdateGlobalState = VulkanRendererUpdateGlobalState;
        OutRendererBackend->UpdateObject      = VulkanUpdateObject;
        OutRendererBackend->EndFrame          = VulkanRendererBackendEndFrame;
        OutRendererBackend->Resized           = VulkanRendererBackendResized;

        OutRendererBackend->CreateTexture     = VulkanCreateTexture;
        OutRendererBackend->DestroyTexture    = VulkanDestroyTexture;

        return true;
    }

    return false;
}

void RendererBackendDestroy(renderer_backend* RendererBackend)
{
    RendererBackend->Initialize        = 0;
    RendererBackend->Shutdown          = 0;
    RendererBackend->BeginFrame        = 0;
    RendererBackend->UpdateGlobalState = 0;
    RendererBackend->UpdateObject      = 0;
    RendererBackend->EndFrame          = 0;
    RendererBackend->Resized           = 0;
}

