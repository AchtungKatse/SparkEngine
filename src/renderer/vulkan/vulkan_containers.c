#include "Spark/renderer/vulkan/vulkan_context.h"
#include "Spark/renderer/vulkan/vulkan_mesh.h"
#include "Spark/renderer/vulkan/vulkan_shader.h"
#include "Spark/renderer/vulkan/vulkan_texture.h"

darray_impl(vulkan_mesh_t,   vulkan_mesh);
darray_impl(vulkan_shader_t, vulkan_shader);
darray_impl(vulkan_image_t,  vulkan_image);
darray_impl(VkDrawIndexedIndirectCommand, VkDrawIndexedIndirectCommand);
darray_impl(indirect_draw_info_t, indirect_draw_info);
