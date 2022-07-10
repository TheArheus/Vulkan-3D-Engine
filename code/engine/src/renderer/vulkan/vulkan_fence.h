#pragma once

#include "vulkan_types.inl"

void VulkanFenceCreate(vulkan_context* Context, b8 CreateSignaled, vulkan_fence* OutFence);
void VulkanFenceDestroy(vulkan_context* Context, vulkan_fence* Fence);

b8 VulkanFenceWait(vulkan_context* Context, vulkan_fence* Fence, u64 TimeoutNs);
void VulkanFenceReset(vulkan_context* Context, vulkan_fence* Fence);
