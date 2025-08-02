#pragma once

#include <vulkan/vulkan_core.h>

struct vulkan_command_buffer;

typedef struct vulkan_queue {
    VkQueue handle;
    u32 family_index;
} vulkan_queue_t;

void vulkan_queue_submit_sync(vulkan_queue_t* queue, struct vulkan_command_buffer* command_buffer);
void vulkan_queue_submit_async(vulkan_queue_t* queue, 
        struct vulkan_command_buffer* command_buffer, 
        VkSemaphoreWaitFlags wait_flags,
        u32 wait_semaphore_count, 
        VkSemaphore wait_semaphores, 
        u32 signal_semaphore_count, 
        VkSemaphore signal_semaphores,
        VkFence fence);

void vulkan_queue_wait_idle(vulkan_queue_t* queue);
