#pragma once

#include "vulkan_types.inl"

b8 VulkanGraphicsPipelineCreate(vulkan_context* Context, vulkan_renderpass* Renderpass, 
                                u32 AttributeCount, VkVertexInputAttributeDescription* Attributes,
                                u32 DescriptorSetLayoutCount, VkDescriptorSetLayout* DescriptorSetLayouts, 
                                u32 StageCount, VkPipelineShaderStageCreateInfo* Stages,
                                VkViewport Viewport, VkRect2D Scissor, b8 IsWireframe, vulkan_pipeline* OutPipeline);

void VulkanGraphicsPipelineDestroy(vulkan_context* Context, vulkan_pipeline* Pipeline);

void VulkanPipelineBind(vulkan_command_buffer* Commandbuffer, VkPipelineBindPoint BindPoint, vulkan_pipeline* Pipeline);
