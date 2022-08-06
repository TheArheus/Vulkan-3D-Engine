#include "renderer_backend.h"

#include "vulkan/vulkan_backend.h"


b8 RendererBackendCreate(renderer_backend_type Type, renderer_backend* OutRendererBackend)
{
    if(Type == RENDERER_BACKEND_TYPE_VULKAN)
    {
        OutRendererBackend->Initialize        = VulkanRendererBackendInitialize;
        OutRendererBackend->Shutdown          = VulkanRendererBackendShutdown;
        OutRendererBackend->DrawGeometry      = VulkanDrawGeometry;
        OutRendererBackend->Resized           = VulkanRendererBackendResized;

        OutRendererBackend->BeginFrame        = VulkanRendererBackendBeginFrame;
        OutRendererBackend->EndFrame          = VulkanRendererBackendEndFrame;

        OutRendererBackend->BeginRenderpass   = VulkanRendererBeginRenderpass;
        OutRendererBackend->EndRenderpass     = VulkanRendererEndRenderpass;

        OutRendererBackend->CreateTexture     = VulkanCreateTexture;
        OutRendererBackend->DestroyTexture    = VulkanDestroyTexture;

        OutRendererBackend->CreateMaterial    = VulkanRendererCreateMaterial;
        OutRendererBackend->DestroyMaterial   = VulkanRendererDestroyMaterial;

        OutRendererBackend->CreateGeometry    = VulkanRendererCreateGeometry;
        OutRendererBackend->DestroyGeometry   = VulkanRendererDestroyGeometry;

        OutRendererBackend->UpdateGlobalWorldState = VulkanRendererUpdateGlobalWorldState;
        OutRendererBackend->UpdateGlobalUiState    = VulkanRendererUpdateGlobalUiState;

        return true;
    }

    return false;
}

void RendererBackendDestroy(renderer_backend* RendererBackend)
{
    RendererBackend->Initialize         = 0;
    RendererBackend->Shutdown           = 0;
    RendererBackend->DrawGeometry       = 0;
    RendererBackend->Resized            = 0;

    RendererBackend->BeginRenderpass    = 0;
    RendererBackend->EndRenderpass      = 0;

    RendererBackend->BeginFrame         = 0;
    RendererBackend->EndFrame           = 0;

    RendererBackend->CreateTexture      = 0;
    RendererBackend->DestroyTexture     = 0;

    RendererBackend->CreateMaterial     = 0;
    RendererBackend->DestroyMaterial    = 0;

    RendererBackend->CreateGeometry     = 0;
    RendererBackend->DestroyGeometry    = 0;

    RendererBackend->UpdateGlobalWorldState = 0;
    RendererBackend->UpdateGlobalUiState    = 0;
}

