#include "Spark/resources/loaders/texture_loader.h"
#include "Spark/containers/generic/darray_ints.h"
#include "Spark/core/sstring.h"
#include "Spark/renderer/renderer_frontend.h"
#include "Spark/renderer/texture.h"
#include "Spark/resources/loaders/loader_utils.h"
#include "Spark/resources/resource_types.h"
// =================================
// Private Data Types
// =================================
darray_impl(texture_t, texture);

typedef struct texture_loader_state {
    darray_u8_t resource_buffer;
    darray_resource_t resources;
    darray_texture_t textures;
} texture_loader_state_t;

static texture_loader_state_t* loader_state;

// =================================
// Private Functions
// =================================

// =================================
// Function Implementations
// =================================
void texture_loader_initialzie(linear_allocator_t* allocator) {
    loader_state = linear_allocator_allocate(allocator, sizeof(texture_loader_state_t));
    darray_u8_create(256, &loader_state->resource_buffer);
    darray_resource_create(256, &loader_state->resources);
    darray_texture_create(256, &loader_state->textures);
}
void texture_loader_shutdown() {
    darray_u8_destroy(&loader_state->resource_buffer);
    darray_resource_destroy(&loader_state->resources);
    darray_texture_destroy(&loader_state->textures);
}

resource_t texture_loader_load_text_resouce(const char* text, u32 length, b8 auto_delete) {
    // Load texture data
    texture_config_t config = {};

    const u32 arg_buffer_count = 8;
    const u32 arg_buffer_size = 48;
    char args[arg_buffer_count][arg_buffer_size];
    u32 text_offset = 0;

    while (text_offset < length) {
        u32 arg_count = 0;
        loader_read_line(text, &text_offset, (char*)args, arg_buffer_count, arg_buffer_size, &arg_count);

        if (string_equal(args[0], "path")) {
            string_ncopy(config.path, args[1], TEXTURE_CONFIG_PATH_MAX_SIZE);
        } else
        if (string_equal(args[0], "filter")) {
            if (string_equali(args[1], "nearest")) {
                config.filter = TEXTURE_FILTER_NEAREST;
            } else if (string_equali(args[1], "linear")) {
                config.filter = TEXTURE_FILTER_LINEAR;
            } else {
                SERROR("Unknown texture sampler mode '%s'", args[1]);
            }
        }
    }

    // Load texture
    texture_t texture = load_texture_from_config(&config);
    u32 texture_index = loader_state->textures.count;
    darray_texture_push(&loader_state->textures, texture);

    resource_t resource = {
        .auto_delete = auto_delete,
        .generation = 0,
        .ref_count = 1,
        .internal_index = texture_index,
        .type = RESOURCE_TYPE_TEXTURE,
    };

    return resource;
}

texture_t* texture_loader_get_texture(u32 index) {
    return &loader_state->textures.data[index];
}

texture_t load_texture_from_config(texture_config_t* config) {
    SDEBUG("Creating vulkan sampler with filter: %d", config->filter);
    SASSERT(string_length(config->path) > 0, "Cannot load texture config with no path.");
    return renderer_create_image_from_path(config->path, config->filter);
}
