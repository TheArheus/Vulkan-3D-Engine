#pragma once

#include "vulkan_types.inl"

void VulkanCommandBufferAllocate(vulkan_context* Context, VkCommandPool Pool, b8 IsPrimary, vulkan_command_buffer* OutCommandBuffer);
void VulkanCommandBufferFree(vulkan_context* Context, VkCommandPool Pool, vulkan_command_buffer* CommandBuffer);

void VulkanCommandBufferBegin(vulkan_command_buffer* CommandBuffer, b8 IsSingleUse, b8 IsRenderPassContinue, b8 IsSimultaneousUse);
void VulkanCommandBufferEnd(vulkan_command_buffer* CommandBuffer);

void VulkanCommandBufferUpdateSubmitted(vulkan_command_buffer* CommandBuffer);
void VulkanCommandBufferReset(vulkan_command_buffer* CommandBuffer);

void VulkanCommandBufferAllocateAndBeginSingleUse(vulkan_context* Context, VkCommandPool Pool, vulkan_command_buffer* OutCommandBuffer);
void VulkanCommandBufferEndSingleUse(vulkan_context* Context, VkCommandPool Pool, vulkan_command_buffer* CommandBuffer, VkQueue Queue);
