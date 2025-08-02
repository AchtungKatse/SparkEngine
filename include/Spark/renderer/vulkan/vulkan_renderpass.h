#pragma once

#include "Spark/renderer/vulkan/vulkan_command_buffer.h"
#include "Spark/renderer/vulkan/vulkan_swapchain.h"
#include <vulkan/vulkan_core.h>
struct vulkan_context;

#define VULKAN_RENDERPASS_MAX_ATTACHMENTS 2
typedef struct vulkan_renderpass {
    VkRenderPass handle;
    VkFramebuffer framebuffers[SWAPCHAIN_MAX_IMAGE_COUNT];
    VkClearValue clear_color;
    b8 use_depth;
} vulkan_renderpass_t;

void vulkan_renderpass_create(struct vulkan_context* context, 
        VkClearValue clear_color, 
        b8 use_depth,
        b8 use_clear,
        u32 width,
        u32 height,
        vulkan_renderpass_t* out_renderpass);
void vulkan_renderpass_destroy(struct vulkan_context* context, vulkan_renderpass_t* renderpass);

void vulkan_renderpass_update_framebuffers(struct vulkan_context* context, vulkan_renderpass_t* renderpass);
void vulkan_renderpass_begin(vulkan_renderpass_t* renderpass, vulkan_command_buffer_t* command_buffer, VkRect2D render_area, u32 image_index);
void vulkan_renderpass_end(vulkan_command_buffer_t* command_buffer);
