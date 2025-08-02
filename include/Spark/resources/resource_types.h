#pragma once

#include "Spark/renderer/material.h"
#include "Spark/renderer/shader.h"
#include "Spark/renderer/texture.h"
#include "Spark/types/s3d.h"
typedef enum resource_type : u8 {
    RESOURCE_TYPE_NULL,
    RESOURCE_TYPE_TEXT,
    RESOURCE_TYPE_MODEL,
    RESOURCE_TYPE_MESH,
    RESOURCE_TYPE_SHADER,
    RESOURCE_TYPE_TEXTURE,
    RESOURCE_TYPE_MATERIAL,
    RESOURCE_TYPE_ENUM_MAX,
} resource_type_t;

typedef struct resource {
    resource_type_t type;
    b8 auto_delete;
    u8 generation;
    u16 ref_count;
    u16 internal_index;
} resource_t;
darray_header(resource_t, resource);

texture_t*  resource_get_texture(resource_t* resource);
shader_t*   resource_get_shader(resource_t* resource);
material_t* resource_get_material(resource_t* resource);
entity_t resource_instance_model(resource_t* resource, u32 material_count, material_t** materials);

typedef struct mesh_resource {
    u32 mesh_index;
} mesh_resource_t;

const static u16 BINARY_RESOURCE_FILE_MAGIC = ('B' << 8 | 'R');
// Configurations

#define TEXTURE_CONFIG_PATH_MAX_SIZE 128
typedef struct texture_config {
    char path[TEXTURE_CONFIG_PATH_MAX_SIZE];
    texture_filter_t filter;
} texture_config_t;
