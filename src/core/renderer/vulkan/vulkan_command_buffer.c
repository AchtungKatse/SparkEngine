#include "Spark/renderer/vulkan/vulkan_command_buffer.h"
#include "Spark/renderer/vulkan/vulkan_context.h"
#include "Spark/renderer/vulkan/vulkan_utils.h"
#include <vulkan/vulkan_core.h>

void vulkan_command_buffer_create(vulkan_context_t* context, u32 queue_family_index, VkCommandPoolCreateFlagBits flags, vulkan_command_buffer_t* out_command_buffer) {
    VkCommandPoolCreateInfo pool_create_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .queueFamilyIndex = queue_family_index,
        .flags = flags,
    };

    vkCreateCommandPool(context->logical_device, &pool_create_info, context->allocator, &out_command_buffer->command_pool);

    VkCommandBufferAllocateInfo allocate_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = out_command_buffer->command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    VK_CHECK(vkAllocateCommandBuffers(context->logical_device, &allocate_info, &out_command_buffer->handle));
}

void vulkan_command_buffer_destroy(vulkan_context_t* context, vulkan_command_buffer_t* command_buffer) {
   vkDestroyCommandPool(context->logical_device, command_buffer->command_pool, context->allocator); 
}

void vulkan_command_buffer_record(vulkan_command_buffer_t* command_buffer, VkCommandBufferUsageFlags usage_flags) {
    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = usage_flags,
        .pInheritanceInfo = NULL,
        .pNext = NULL,
    };

    VK_CHECK(vkBeginCommandBuffer(command_buffer->handle, &begin_info));
}

void vulkan_command_buffer_end_recording(vulkan_command_buffer_t* command_buffer) {
    VK_CHECK(vkEndCommandBuffer(command_buffer->handle));
}

void vulkan_command_buffer_end_and_submit_sync(vulkan_command_buffer_t* command_buffer, struct vulkan_queue* queue) {
    vulkan_command_buffer_end_recording(command_buffer);
    vulkan_queue_submit_sync(queue, command_buffer);
    vulkan_queue_wait_idle(queue);
}
