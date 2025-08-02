#include "Spark/renderer/vulkan/vulkan_semaphor.h"
#include "Spark/renderer/vulkan/vulkan_utils.h"
#include "Spark/renderer/vulkan/vulkan_context.h"
#include <vulkan/vulkan_core.h>

void vulkan_semaphor_create(struct vulkan_context* context, VkSemaphore* out_semaphore) {
    VkSemaphoreTypeCreateInfo semaphore_type_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
        .semaphoreType = VK_SEMAPHORE_TYPE_BINARY,
        .initialValue = 0,
    };

    VkSemaphoreCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = &semaphore_type_info
    };

    VK_CHECK(vkCreateSemaphore(context->logical_device, &create_info, context->allocator, out_semaphore));
}

void vulkan_semaphor_destroy(struct vulkan_context* context, VkSemaphore semaphore) {
    vkDestroySemaphore(context->logical_device, semaphore, context->allocator);
}
