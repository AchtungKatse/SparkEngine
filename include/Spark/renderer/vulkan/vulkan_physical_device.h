#pragma once

#include <vulkan/vulkan_core.h>
typedef struct vulkan_physical_device {
    VkPhysicalDevice handle;
    VkPhysicalDeviceMemoryProperties memory_properties;
    VkFormat depth_format;
} vulkan_physical_device_t;
