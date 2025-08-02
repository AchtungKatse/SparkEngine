#pragma once

#include <vulkan/vulkan_core.h>
struct vulkan_context;

void vulkan_semaphor_create(struct vulkan_context* context, VkSemaphore* out_semaphore);
void vulkan_semaphor_destroy(struct vulkan_context* context, VkSemaphore semaphore);
