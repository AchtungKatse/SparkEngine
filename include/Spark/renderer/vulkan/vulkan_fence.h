#pragma once

#include <vulkan/vulkan_core.h>
struct vulkan_context;

void vulkan_fence_create(struct vulkan_context* context, VkFence* out_fence);
void vulkan_fence_destroy(struct vulkan_context* context, VkFence fence);
