#pragma once

#include "Spark/renderer/vulkan/vulkan_texture.h"
#include <vulkan/vulkan_core.h>
#define SWAPCHAIN_MAX_IMAGE_COUNT 4

typedef struct {
    u32 image_count;
    VkSwapchainKHR handle;

    VkImage images[SWAPCHAIN_MAX_IMAGE_COUNT];
    VkImageView image_views[SWAPCHAIN_MAX_IMAGE_COUNT];

    vulkan_image_t depth_buffers[SWAPCHAIN_MAX_IMAGE_COUNT];

    VkFormat image_format;
} vulkan_swapchain_t;

struct vulkan_context;

void vulkan_swapchain_create(struct vulkan_context* context, vulkan_swapchain_t* out_swapchain);
void vulkan_swapchain_destroy(struct vulkan_context* context, vulkan_swapchain_t* swapchain);
void vulkan_swapchain_recreate(struct vulkan_context* context, vulkan_swapchain_t* swapchain);
