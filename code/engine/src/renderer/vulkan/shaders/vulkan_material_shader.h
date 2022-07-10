#pragma once

#include "renderer/vulkan/vulkan_types.inl"
#include "renderer/renderer_types.inl"

b8 VulkanMaterialShaderCreate(vulkan_context* Context, vulkan_material_shader* OutShader);
void VulkanMaterialShaderDestroy(vulkan_context* Context, vulkan_material_shader* Shader);
void VulkanMaterialShaderUse(vulkan_context* Context, vulkan_material_shader* Shader);

void VulkanMaterialShaderUpdateGlobalState(vulkan_context* Context, vulkan_material_shader* Shader, r32 DeltaTime);
void VulkanMaterialShaderUpdateObject(vulkan_context* Context, vulkan_material_shader* Shader, geometry_render_data RenderData);

b8 VulkanMaterialShaderAcquireResources(vulkan_context* Context, vulkan_material_shader* Shader, u32* OutObjectID);
void VulkanMaterialShaderReleaseResources(vulkan_context* Context, vulkan_material_shader* Shader, u32 ObjectID);
