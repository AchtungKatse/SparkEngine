#include "Spark/renderer/vulkan/vulkan_texture.h"
#include "Spark/renderer/texture.h"
#include "Spark/renderer/vulkan/vulkan_buffer.h"
#include "Spark/renderer/vulkan/vulkan_command_buffer.h"
#include "Spark/renderer/vulkan/vulkan_context.h"
#include "Spark/renderer/vulkan/vulkan_queue.h"
#include "Spark/renderer/vulkan/vulkan_utils.h"
#include <vulkan/vulkan_core.h>

// TODO: Temporary, Use custom image importer if possible
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// TODO: End temporary

//==============================
// Private functions
//==============================
u32 get_bytes_per_pixel(VkFormat format);
void create_sampler(vulkan_context_t* context, VkFilter min_filter, VkFilter max_filter, VkSamplerAddressMode address_mode, VkSampler* out_sampler);

void vulkan_image_create_from_file(struct vulkan_context* context, const char* path, texture_filter_t filter_mode, VkImageUsageFlags usage, VkMemoryPropertyFlags memory_properties, vulkan_image_t* out_image) {
    s32 width = 0;
    s32 height = 0;
    s32 components = 0;
    u8* pixels = stbi_load(path, &width, &height, &components, 4);

    SASSERT(pixels != NULL, "Failed to load image '%s'", path);

    vulkan_image_create(context, filter_mode, VK_FORMAT_R8G8B8A8_SRGB, width, height, usage, memory_properties, out_image);
    vulkan_image_upload_pixels(context, VK_FORMAT_R8G8B8A8_SRGB, width, height, pixels, out_image);

    stbi_image_free(pixels);
}

void vulkan_image_create(struct vulkan_context* context, texture_filter_t filter_mode, VkFormat format, u32 width, u32 height, VkImageUsageFlags usage, VkMemoryPropertyFlags memory_properties, vulkan_image_t* out_image) {
    // Create image
    VkImageCreateInfo create_info = {
        .sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .flags                 = 0,
        .imageType             = VK_IMAGE_TYPE_2D,
        .format                = format,
        .extent                = {
            .width             = width,
            .height            = height,
            .depth             = 1,
        },
        .mipLevels             = 1,
        .arrayLayers           = 1,
        .samples               = VK_SAMPLE_COUNT_1_BIT, 
        .tiling                = VK_IMAGE_TILING_OPTIMAL,
        .usage                 = usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices   = NULL,
        .initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    VK_CHECK(vkCreateImage(context->logical_device, &create_info, context->allocator, &out_image->handle));

    // Get memory requirements
    VkMemoryRequirements memory_requirements;
    vkGetImageMemoryRequirements(context->logical_device, out_image->handle, &memory_requirements);

    // Allocate memory
    u32 memory_type_index = vulkan_get_memory_index(context, memory_requirements.memoryTypeBits, memory_properties);

    VkMemoryAllocateInfo allocate_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .memoryTypeIndex = memory_type_index,
        .allocationSize = memory_requirements.size,
    };

    VK_CHECK(vkAllocateMemory(context->logical_device, &allocate_info, context->allocator, &out_image->memory));

    // Bind memory
    VK_CHECK(vkBindImageMemory(context->logical_device, out_image->handle, out_image->memory, 0));

    // Create image view
    out_image->aspect_flags = vulkan_image_usage_to_aspect(usage);
    vulkan_image_view_create(context, out_image->handle, VK_IMAGE_VIEW_TYPE_2D, format, out_image->aspect_flags, 1, 1, &out_image->view);

    // Create sampler
    VkFilter filter;
    switch (filter_mode) {
        case TEXTURE_FILTER_LINEAR:
            filter = VK_FILTER_LINEAR;
            break;
        case TEXTURE_FILTER_NEAREST:
            filter = VK_FILTER_NEAREST;
            break;
    }

    create_sampler(context, filter, filter, VK_SAMPLER_ADDRESS_MODE_REPEAT, &out_image->sampler);
}

void vulkan_image_destroy(struct vulkan_context* context, vulkan_image_t* texture) {
    vkDestroySampler(context->logical_device, texture->sampler, context->allocator);
    vkDestroyImageView(context->logical_device, texture->view, context->allocator);

    vkFreeMemory(context->logical_device, texture->memory, context->allocator);
    vkDestroyImage(context->logical_device, texture->handle, context->allocator);
}

// TODO: the image transition and copy functions should not have any waiting but they do because they wait for the queues to finish
void vulkan_image_upload_pixels(struct vulkan_context* context, VkFormat format, u32 width, u32 height, const void* pixel_data, vulkan_image_t* image) {
    // Get image size
    u32 bytes_per_pixel = get_bytes_per_pixel(format);

    VkDeviceSize layer_size = width * height * bytes_per_pixel;
    const u32 layer_count = 1;
    VkDeviceSize image_size = layer_size * layer_count;

    // Create staging buffer
    vulkan_buffer_t staging;
    vulkan_buffer_create(context, image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &context->transfer_queue, &staging);

    vulkan_buffer_update(context, &staging, pixel_data, image_size, 0);

    // Set image layout
    vulkan_image_transition_layout(context, 
            image, 
            VK_IMAGE_LAYOUT_UNDEFINED, 
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
            VK_QUEUE_FAMILY_IGNORED, 
            VK_QUEUE_FAMILY_IGNORED, 
            &context->copy_command_buffer,
            &context->transfer_queue);
    vulkan_buffer_copy_to_image(context, image, &staging, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, width, height);
    vulkan_image_transition_layout(context, 
            image, 
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 
            VK_QUEUE_FAMILY_IGNORED, 
            VK_QUEUE_FAMILY_IGNORED, 
            &context->copy_to_graphics_command_buffer,
            &context->graphics_queue);


    vulkan_buffer_destroy(context, &staging);
}

void vulkan_image_transition_layout(struct vulkan_context* context, 
        vulkan_image_t* image, 
        VkImageLayout from, 
        VkImageLayout to, 
        u32 src_queue_index, 
        u32 dest_queue_index, 
        vulkan_command_buffer_t* command_buffer, 
        vulkan_queue_t* queue) {
    // Record
    vulkan_command_buffer_record(command_buffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .oldLayout = from,
        .newLayout = to,
        .srcQueueFamilyIndex = src_queue_index,
        .dstQueueFamilyIndex = dest_queue_index,
        .image = image->handle,
        .subresourceRange = {
            .aspectMask = image->aspect_flags,
            .baseMipLevel = 0,
            .layerCount = 1,
            .levelCount = 1,
            .baseArrayLayer = 0,
        },
    };

    VkPipelineStageFlagBits source_stage = 0;
    VkPipelineStageFlagBits destination_stage = 0;

    if (from == VK_IMAGE_LAYOUT_UNDEFINED && to == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (from == VK_IMAGE_LAYOUT_UNDEFINED && to == VK_IMAGE_LAYOUT_GENERAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }

    if (from == VK_IMAGE_LAYOUT_UNDEFINED && 
            to == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } /* Convert back from read-only to updateable */
    else if (from == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && to == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        source_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } /* Convert from updateable texture to shader read-only */
    else if (from == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && 
            to == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } /* Convert depth texture from undefined state to depth-stencil buffer */
    else if (from == VK_IMAGE_LAYOUT_UNDEFINED && to == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    } /* Wait for render pass to complete */
    else if (from == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && to == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = 0; // VK_ACCESS_SHADER_READ_BIT;
        barrier.dstAccessMask = 0;
        /*
           source_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        ///		destination_stage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
        destination_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        */
        source_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } /* Convert back from read-only to color attachment */
    else if (from == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && to == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        source_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        destination_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    } /* Convert from updateable texture to shader read-only */
    else if (from == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && to == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        source_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } /* Convert back from read-only to depth attachment */
    else if (from == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && to == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        source_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        destination_stage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    } /* Convert from updateable depth texture to shader read-only */
    else if (from == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL && to == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        source_stage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }

    vkCmdPipelineBarrier(command_buffer->handle, source_stage, destination_stage,
            0, 0, NULL, 0, NULL, 1, &barrier);   

    vulkan_command_buffer_end_and_submit_sync(command_buffer, queue);
}

//==============================
// Private functions
//==============================
u32 get_bytes_per_pixel(VkFormat format) {
    switch (format) {
        default:
            SCRITICAL("Failed to get number of bytes per pixel for format 0x%x", format);
            return 0;
        case VK_FORMAT_R8G8B8_SNORM:
            return 3;
        case VK_FORMAT_R8G8B8_UNORM:
            return 3;
        case VK_FORMAT_R8G8B8_UINT:
            return 3;
        case VK_FORMAT_R8G8B8_SRGB:
            return 3;
        case VK_FORMAT_R8G8B8A8_SNORM:
            return 4;
        case VK_FORMAT_R8G8B8A8_UNORM:
            return 4;
        case VK_FORMAT_R8G8B8A8_UINT:
            return 4;
        case VK_FORMAT_R8G8B8A8_SRGB:
            return 4;
    }
}

void create_sampler(vulkan_context_t* context, VkFilter min_filter, VkFilter max_filter, VkSamplerAddressMode address_mode, VkSampler* out_sampler) {
    VkSamplerCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .flags = 0,
        .minFilter = min_filter,
        .magFilter = max_filter,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = address_mode,
        .addressModeV = address_mode,
        .addressModeW = address_mode,
        .mipLodBias = 0.0f,
        .anisotropyEnable = VK_FALSE,
        .maxAnisotropy = 1,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0f,
        .maxLod = 0.0f,
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,
    };

    VK_CHECK(vkCreateSampler(context->logical_device, &create_info, context->allocator, out_sampler));
}

VkImageAspectFlags vulkan_image_usage_to_aspect(VkImageUsageFlags usage) {
    VkImageAspectFlags aspect_flags = 0;
    if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        aspect_flags |= VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) {
        aspect_flags |= VK_IMAGE_ASPECT_COLOR_BIT;
    }
    if (usage & VK_IMAGE_USAGE_SAMPLED_BIT) {
        aspect_flags |= VK_IMAGE_ASPECT_COLOR_BIT;
    }

    SASSERT(aspect_flags != 0, "Failed to get aspect flags from image usage: 0x%x", usage);

    return aspect_flags;
}
