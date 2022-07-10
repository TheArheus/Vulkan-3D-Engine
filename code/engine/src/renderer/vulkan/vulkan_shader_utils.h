#pragma once

#include "vulkan_types.inl"

b8 CreateShaderModule(vulkan_context* Context, const char* Name, const char* TypeStr, VkShaderStageFlagBits ShaderStageBit, u32 StageIndex, vulkan_shader_stage* ShaderStages);

