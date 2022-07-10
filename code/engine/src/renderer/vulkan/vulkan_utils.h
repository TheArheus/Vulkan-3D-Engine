#pragma once

#include "vulkan_types.inl"

const char* VulkanResultString(VkResult Result, b8 GetExtended);
b8 VulkanResultIsSuccess(VkResult Result);
