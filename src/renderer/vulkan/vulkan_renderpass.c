#include "Spark/renderer/vulkan/vulkan_renderpass.h"
#include "Spark/core/smemory.h"
#include "Spark/renderer/vulkan/vulkan_command_buffer.h"
#include "Spark/renderer/vulkan/vulkan_context.h"
#include "Spark/renderer/vulkan/vulkan_framebuffer.h"
#include "Spark/renderer/vulkan/vulkan_swapchain.h"
#include "Spark/renderer/vulkan/vulkan_utils.h"
#include <vulkan/vulkan_core.h>

void vulkan_renderpass_create(struct vulkan_context* context, 
        VkClearValue clear_color, 
        b8 use_depth,
        b8 use_clear,
        u32 width,
        u32 height,
        vulkan_renderpass_t* out_renderpass) {
    // Set out_renderpass values
    out_renderpass->clear_color = clear_color;
    out_renderpass->use_depth = use_depth;

    // Create vulkan renderpass object
    u32 attachment_count = 1;
    VkAttachmentDescription attachments[VULKAN_RENDERPASS_MAX_ATTACHMENTS];
    attachments[0] = (VkAttachmentDescription) {
        .flags = 0,
        .format = context->swapchain.image_format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = (use_clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE),
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    VkAttachmentReference attachment_reference = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };


    VkAttachmentReference depth_attachment_reference = {
        .attachment = attachment_count,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    VkSubpassDescription subpass_description = {
        .flags = 0,
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount = 0,
        .pInputAttachments = NULL,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attachment_reference,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments = NULL,
        .pResolveAttachments = NULL,
    };

    if (use_depth) {
        attachments[attachment_count++] = (VkAttachmentDescription) {
            .flags = 0,
                .format = context->physical_device.depth_format,
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        };
        subpass_description.pDepthStencilAttachment = &depth_attachment_reference;
    }

    VkRenderPassCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .attachmentCount = attachment_count,
        .pAttachments = attachments,
        .subpassCount = 1,
        .pSubpasses = &subpass_description,
        .dependencyCount = 0,
        .pDependencies = NULL
    };
    VK_CHECK(vkCreateRenderPass(context->logical_device, &create_info, context->allocator, &out_renderpass->handle));

    // Create framebuffers
    vulkan_renderpass_update_framebuffers(context, out_renderpass);
}

void vulkan_renderpass_update_framebuffers(vulkan_context_t* context, vulkan_renderpass_t* renderpass) {
    VkImageView framebuffer_attachments[VULKAN_RENDERPASS_MAX_ATTACHMENTS];

    // HACK: This should really be stored with the renderpass
    u32 attachment_count = 1;
    if (renderpass->use_depth) {
        attachment_count++;
    }

    for (u32 i = 0; i < SWAPCHAIN_MAX_IMAGE_COUNT; i++) {
        if (renderpass->framebuffers[i] != VK_NULL_HANDLE) {
            vulkan_framebuffer_destroy(context, renderpass->framebuffers[i]);
        } 
    }

    for (u32 i = 0; i < context->swapchain.image_count; i++) {
        framebuffer_attachments[0] = context->swapchain.image_views[i];
        if (renderpass->use_depth) {
            framebuffer_attachments[1] = context->swapchain.depth_buffers[i].view;
        }
        vulkan_framebuffer_create(context, 
                renderpass->handle, 
                attachment_count,
                framebuffer_attachments,
                context->screen_width,
                context->screen_height,
                &renderpass->framebuffers[i]);
    }
}

void vulkan_renderpass_destroy(struct vulkan_context* context, vulkan_renderpass_t* renderpass) {
    for (u32 i = 0; i < context->swapchain.image_count; i++) {
        vkDestroyFramebuffer(context->logical_device, renderpass->framebuffers[i], context->allocator);
    }
    vkDestroyRenderPass(context->logical_device, renderpass->handle, context->allocator);
}

void vulkan_renderpass_begin(vulkan_renderpass_t* renderpass, vulkan_command_buffer_t* command_buffer, VkRect2D render_area, u32 image_index) {
    VkClearValue clear_colors[16] = {};
    clear_colors[0] = renderpass->clear_color;

    u32 clear_count = 1;
    if (renderpass->use_depth) {
        VkClearValue clear_color = {
            .depthStencil = {
                .depth = 1.0f,
                .stencil = 0,
            },
        };
        clear_colors[clear_count++] = clear_color;
    }
    VkRenderPassBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = NULL,
        .renderPass = renderpass->handle,
        .framebuffer = renderpass->framebuffers[image_index],
        .renderArea = render_area,
        .clearValueCount = clear_count,
        .pClearValues = clear_colors,
    };

    vkCmdBeginRenderPass(command_buffer->handle, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
}

void vulkan_renderpass_end(vulkan_command_buffer_t* command_buffer) {
    vkCmdEndRenderPass(command_buffer->handle);
}
