#pragma once

#include "Spark/renderer/material.h"
#include "Spark/renderer/shader.h"
#include "Spark/types/s3d.h"
#include <vulkan/vulkan_core.h>

struct vulkan_context;
struct vulkan_renderpass;
struct vulkan_mesh;
struct shader_resource;
struct vulkan_buffer;
struct vulkan_command_buffer;

#define VULKAN_SHADER_MAX_STAGES 2
#define VULKAN_SHADER_MAX_SETS 4
#define VULKAN_SHADER_MAX_RESOURCES 4
typedef struct vulkan_shader {
    VkShaderModule vert;
    VkShaderModule frag;
    VkPipeline pipeline;
    VkPipelineLayout pipeline_layout;
    VkDescriptorSetLayout descriptor_layouts[VULKAN_SHADER_MAX_SETS];
    VkDescriptorSet descriptor_sets[VULKAN_SHADER_MAX_SETS];
    VkWriteDescriptorSet descriptor_writes[VULKAN_SHADER_MAX_RESOURCES];
    u32 resource_count;
    u32 set_count;
#ifdef SPARK_DEBUG
    u32 vertex_stride;
#endif
} vulkan_shader_t;

darray_header(vulkan_shader_t, vulkan_shader);

void vulkan_shader_module_create_from_file(struct vulkan_context* context, const char* path, VkShaderModule* out_module);
void vulkan_shader_module_create(struct vulkan_context* context, const u32* spirv, u32 spirv_size, VkShaderModule* out_module);

void vulkan_shader_create(struct vulkan_context* context, 
        struct vulkan_renderpass* renderpass, 
        u32 subpass, 
        u32 vertex_attribute_count, 
        vertex_attributes_t* vertex_attributes, 
        u32 resource_count, 
        shader_resource_layout_t* resources, 
        u8 use_depth, 
        vulkan_shader_t* out_shader);
void vulkan_shader_destroy(struct vulkan_context* context, const vulkan_shader_t* shader);

void vulkan_shader_bind(vulkan_shader_t* shader, struct vulkan_command_buffer* command_buffer);
void vulkan_shader_bind_resources(struct vulkan_context* context, vulkan_shader_t* shader, u32 binding_start, u32 resource_count, shader_resource_t* resources);
