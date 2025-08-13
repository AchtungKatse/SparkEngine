#include "Spark/renderer/vulkan/vulkan_queue.h"
#include "Spark/renderer/vulkan/vulkan_command_buffer.h"
#include "Spark/renderer/vulkan/vulkan_utils.h"
#include <vulkan/vulkan_core.h>

void vulkan_queue_submit_sync(vulkan_queue_t* queue, struct vulkan_command_buffer* command_buffer) {

    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &command_buffer->handle,
    };

    VK_CHECK(vkQueueSubmit(queue->handle, 1, &submit_info, NULL));
}

void vulkan_queue_submit_async(vulkan_queue_t* queue, 
        vulkan_command_buffer_t* command_buffer, 
        VkSemaphoreWaitFlags wait_flags,
        u32 wait_semaphore_count, 
        VkSemaphore wait_semaphores, 
        u32 signal_semaphore_count, 
        VkSemaphore signal_semaphores,
        VkFence fence) {

    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = wait_semaphore_count,
        .pWaitSemaphores = &wait_semaphores,
        .pWaitDstStageMask = &wait_flags,
        .commandBufferCount = 1,
        .pCommandBuffers = &command_buffer->handle,
        .signalSemaphoreCount = signal_semaphore_count,
        .pSignalSemaphores = &signal_semaphores,
    };

    VK_CHECK(vkQueueSubmit(queue->handle, 1, &submit_info, fence));
}

void vulkan_queue_wait_idle(vulkan_queue_t* queue) {
    vkQueueWaitIdle(queue->handle);
}
