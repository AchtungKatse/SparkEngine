#pragma once

#include <vulkan/vulkan_core.h>

struct vulkan_context;
struct vulkan_queue;

typedef struct vulkan_command_buffer {
    VkCommandBuffer handle;
    VkCommandPool command_pool;
} vulkan_command_buffer_t;

void vulkan_command_buffer_create(struct vulkan_context* context, u32 queue_family_index, VkCommandPoolCreateFlagBits flags, vulkan_command_buffer_t* out_command_buffer);
void vulkan_command_buffer_destroy(struct vulkan_context* context, vulkan_command_buffer_t* command_buffer);

void vulkan_command_buffer_record(vulkan_command_buffer_t* command_buffer, VkCommandBufferUsageFlags usage_flags);
void vulkan_command_buffer_end_recording(vulkan_command_buffer_t* command_buffer);
void vulkan_command_buffer_end_and_submit_sync(vulkan_command_buffer_t* command_buffer, struct vulkan_queue* queue);
