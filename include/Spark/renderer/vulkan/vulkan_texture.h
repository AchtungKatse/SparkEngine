#pragma once
#include "Spark/renderer/texture.h"
#include <vulkan/vulkan_core.h>
#include <Spark/containers/darray.h>

struct vulkan_context;
struct vulkan_queue;
struct vulkan_command_buffer;

typedef struct vulkan_image {
    VkImage handle;
    VkDeviceMemory memory;
    VkImageView view;
    VkSampler sampler; // NOTE: Should probably be its own class
    VkImageAspectFlags aspect_flags;
} vulkan_image_t;

darray_header(vulkan_image_t, vulkan_image);

void vulkan_image_create_from_file(struct vulkan_context* context, const char* path, texture_filter_t filter, VkImageUsageFlags usage, VkMemoryPropertyFlags memory_properties, vulkan_image_t* out_image);
void vulkan_image_create(struct vulkan_context* context, texture_filter_t filter_mode, VkFormat format, u32 width, u32 height, VkImageUsageFlags usage, VkMemoryPropertyFlags memory_properties, vulkan_image_t* out_image);
void vulkan_image_destroy(struct vulkan_context* context, vulkan_image_t* texture);

void vulkan_image_upload_pixels(struct vulkan_context* context, VkFormat format, u32 width, u32 height, const void* pixel_data, vulkan_image_t* image);
void vulkan_image_transition_layout(struct vulkan_context* context, 
        vulkan_image_t* image, 
        VkImageLayout from, 
        VkImageLayout to, 
        u32 src_queue_index, 
        u32 dest_queue_index, 
        struct vulkan_command_buffer* command_buffer, 
        struct vulkan_queue* queue);

VkImageAspectFlags vulkan_image_usage_to_aspect(VkImageUsageFlags usage);
