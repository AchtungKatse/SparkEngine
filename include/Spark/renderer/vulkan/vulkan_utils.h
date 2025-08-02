#pragma once

#include <vulkan/vulkan_core.h>

struct vulkan_context;

#define VK_CHECK(result) { \
                            VkResult __result_internal__ = result; \
                            if (!vulkan_result_is_success(__result_internal__)) { \
                                SERROR("Vulkan expression '" #result "' failed: '%s'.", vulkan_result_string(__result_internal__, true)); \
                            } \
                         }

const char* vulkan_result_string(VkResult result, b8 get_extended);

b8 vulkan_result_is_success(VkResult result);
u32 vulkan_get_memory_index(struct vulkan_context* context, u32 required_memory_types, VkMemoryPropertyFlags required_properties);
void vulkan_image_view_create(struct vulkan_context* context, 
        VkImage image, 
        VkImageViewType view_type, 
        VkFormat format, 
        VkImageAspectFlags aspect_mask, 
        u32 layer_count, 
        u32 mip_levels, 
        VkImageView* out_image_view);
