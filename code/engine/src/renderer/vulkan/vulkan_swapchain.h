#pragma once

#include "vulkan_types.inl"

void VulkanSwapchainCreate(vulkan_context* Context, u32 Width, u32 Height, vulkan_swapchain* OutSwapchain);
void VulkanSwapchainRecreate(vulkan_context* Context, u32 Width, u32 Height, vulkan_swapchain* OutSwapchain);
void VulkanSwapchainDestroy(vulkan_context* Context, vulkan_swapchain* Swapchain);
b8 VulkanSwapchainAcquireNextImageIndex(vulkan_context* Context, vulkan_swapchain* Swapchain, u64 TimeoutMs, VkSemaphore ImageAvailableSemaphore, VkFence Fence, u32* OutImageIndex);
void VulkanSwapchainPresent(vulkan_context* Context, vulkan_swapchain* Swapchain, VkQueue GraphicsQueue, VkQueue PresentQueue, VkSemaphore RenderCompleteSemaphore, u32 PresentImageIndex);
