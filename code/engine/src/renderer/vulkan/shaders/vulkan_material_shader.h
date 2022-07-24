#pragma once

#include "renderer/vulkan/vulkan_types.inl"
#include "renderer/renderer_types.inl"

b8 VulkanMaterialShaderCreate(vulkan_context* Context, vulkan_material_shader* OutShader);
void VulkanMaterialShaderDestroy(vulkan_context* Context, vulkan_material_shader* Shader);
void VulkanMaterialShaderUse(vulkan_context* Context, vulkan_material_shader* Shader);

void VulkanMaterialShaderUpdateGlobalState(vulkan_context* Context, vulkan_material_shader* Shader, r32 DeltaTime);

void VulkanMaterialShaderSetModel(vulkan_context* Context, struct vulkan_material_shader* Shader, mat4 Model);
void VulkanMaterialShaderApplyMaterial(vulkan_context* Context, struct vulkan_material_shader* Shader, material* Material);

b8 VulkanMaterialShaderAcquireResources(vulkan_context* Context, vulkan_material_shader* Shader, material* Material);
void VulkanMaterialShaderReleaseResources(vulkan_context* Context, vulkan_material_shader* Shader, material* Material);
