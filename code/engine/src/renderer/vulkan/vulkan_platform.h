#pragma once

#include "defines.h"

struct platform_state;
struct vulkan_context;

b8 PlatformCreateVulkanSurface(struct vulkan_context* Context);

void PlatformGetRequiredExtensionNames(const char*** ExtensionNames);
