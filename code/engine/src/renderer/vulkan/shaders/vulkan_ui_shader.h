#pragma once

#include "renderer/vulkan/vulkan_types.inl"
#include "renderer/renderer_types.inl"

b8 VulkanUiShaderCreate(vulkan_context* Context, vulkan_ui_shader* OutShader);
void VulkanUiShaderDestroy(vulkan_context* Context, vulkan_ui_shader* Shader);
void VulkanUiShaderUse(vulkan_context* Context, vulkan_ui_shader* Shader);

void VulkanUiShaderUpdateGlobalState(vulkan_context* Context, vulkan_ui_shader* Shader, r32 DeltaTime);

void VulkanUiShaderSetModel(vulkan_context* Context, struct vulkan_ui_shader* Shader, mat4 Model);
void VulkanUiShaderApplyMaterial(vulkan_context* Context, struct vulkan_ui_shader* Shader, material* Material);

b8 VulkanUiShaderAcquireResources(vulkan_context* Context, vulkan_ui_shader* Shader, material* Material);
void VulkanUiShaderReleaseResources(vulkan_context* Context, vulkan_ui_shader* Shader, material* Material);

