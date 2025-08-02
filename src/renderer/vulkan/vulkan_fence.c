#include "Spark/renderer/vulkan/vulkan_fence.h"
#include "Spark/renderer/vulkan/vulkan_context.h"
#include <vulkan/vulkan_core.h>

void vulkan_fence_create(struct vulkan_context* context, VkFence* out_fence) {
    VkFenceCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };
    vkCreateFence(context->logical_device, &create_info, context->allocator, out_fence);
}

void vulkan_fence_destroy(struct vulkan_context* context, VkFence fence) {
    vkDestroyFence(context->logical_device, fence, context->allocator);
}
