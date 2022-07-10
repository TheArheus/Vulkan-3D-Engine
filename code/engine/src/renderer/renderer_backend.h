#pragma once

#include "renderer_types.inl"

b8 RendererBackendCreate(renderer_backend_type Type, renderer_backend* OutRendererBackend);
void RendererBackendDestroy(renderer_backend* RendererBackend);
