#pragma once

#include "vulkan_types.inl"

b8 VulkanDeviceCreate(vulkan_context* Context);
void VulkanDeviceDestroy(vulkan_context* Context);
void VulkanDeviceQuerySwapchainSupport(VkPhysicalDevice PhysicalDevice, VkSurfaceKHR Surface, vulkan_swapchain_support_info* OutSupportInfo);
b8 VulkanDeviceDetectDepthFormat(vulkan_device* Device);
