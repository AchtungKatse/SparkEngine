#include "Spark/renderer/vulkan/vulkan_framebuffer.h"
#include "Spark/renderer/vulkan/vulkan_context.h"
#include "Spark/renderer/vulkan/vulkan_utils.h"
#include <vulkan/vulkan_core.h>

void vulkan_framebuffer_create(struct vulkan_context* context, VkRenderPass renderpass, u32 attachment_count, VkImageView* image_views, u32 width, u32 height, VkFramebuffer* out_framebuffer) {
    // TODO: Remove all hardcoded values
    VkFramebufferCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .renderPass = renderpass,
        .attachmentCount = attachment_count,
        .pAttachments = image_views,
        .width = width,
        .height = height,
        .layers = 1,
    };

    vkCreateFramebuffer(context->logical_device, &create_info, context->allocator, 
            out_framebuffer);
}

void vulkan_framebuffer_destroy(struct vulkan_context* context, VkFramebuffer framebuffer) {
    vkDestroyFramebuffer(context->logical_device, framebuffer, context->allocator);
}

