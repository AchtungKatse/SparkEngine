#pragma once

#include <vulkan/vulkan_core.h>

struct vulkan_context;

void vulkan_framebuffer_create(struct vulkan_context* context, VkRenderPass renderpass, u32 attachment_count, VkImageView* image_views, u32 width, u32 height, VkFramebuffer* out_framebuffer);
void vulkan_framebuffer_destroy(struct vulkan_context* context, VkFramebuffer framebuffer);
