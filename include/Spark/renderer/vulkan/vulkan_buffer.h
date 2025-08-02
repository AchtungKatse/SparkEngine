#pragma once

#include <vulkan/vulkan_core.h>
struct vulkan_context;
struct vulkan_queue;
struct vulkan_image;

typedef enum vulkan_buffer_type {
    VULKAN_BUFFER_TYPE_STORAGE, 
    VULKAN_BUFFER_TYPE_UNIFORM,
} vulkan_buffer_type_t;

typedef struct vulkan_buffer {
    VkBuffer handle;
    VkDeviceMemory memory;
    u32 allocation_size;
    vulkan_buffer_type_t type;
} vulkan_buffer_t;

void vulkan_buffer_create(struct vulkan_context* context, 
        u32 size, 
        VkBufferUsageFlags usage, 
        VkMemoryPropertyFlags properties, 
        struct vulkan_queue* queue, 
        vulkan_buffer_t* out_buffer);

void vulkan_buffer_destroy(struct vulkan_context* context, vulkan_buffer_t* buffer);

void vulkan_buffer_copy(struct vulkan_context* context, vulkan_buffer_t* dest, vulkan_buffer_t* source, u32 size);
void vulkan_buffer_copy_to_image(struct vulkan_context* context, struct vulkan_image* dest, vulkan_buffer_t* source, VkImageLayout image_layout, u32 width, u32 height);
/**
 * @brief Used to set the data of a buffer when it is not host visible.
 */
void vulkan_buffer_set_data(struct vulkan_context* context, vulkan_buffer_t* buffer, void* data, u32 size);

/**
 * @brief Used to set the buffer data when it is visible to the host
 *
 * @param context vulkan_context
 * @param buffer the buffer to be updated
 * @param data The data that will be copied to the buffer
 * @param size The size in bytes of the data
 */
void vulkan_buffer_update(struct vulkan_context* context, vulkan_buffer_t* buffer, const void* data, u32 size, u32 offset);

void vulkan_buffer_create_descriptor_write(vulkan_buffer_t* buffer, u32 binding_index, VkDescriptorSet descriptor_set, VkDescriptorType type, VkDescriptorBufferInfo* buffer_info, VkWriteDescriptorSet* write);
