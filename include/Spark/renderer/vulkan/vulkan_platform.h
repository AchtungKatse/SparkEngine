#pragma once

struct platform_state;
struct VkInstance_T;
struct VkAllocationCallbacks;
struct VkSurfaceKHR_T;

void platform_get_required_extension_names(const char** names, u32 start_index, u32* out_extension_count);
b8 platform_create_vulkan_surface(
        struct platform_state* plat_state,
        struct VkInstance_T* instance,
        struct VkAllocationCallbacks* allocator,
        struct VkSurfaceKHR_T** out_surface);
