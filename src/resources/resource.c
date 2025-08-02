#include "Spark/core/logging.h"
#include "Spark/resources/loaders/material_loader.h"
#include "Spark/resources/loaders/model_loader.h"
#include "Spark/resources/loaders/shader_loader.h"
#include "Spark/resources/loaders/texture_loader.h"
#include "Spark/resources/resource_types.h"

texture_t* resource_get_texture(resource_t* resource) {
    SASSERT(resource, "Cannot load null resource data");
    SASSERT(resource->type == RESOURCE_TYPE_TEXTURE, "Cannot load texture from resource type. Resource is not a texture");
    return texture_loader_get_texture(resource->internal_index);
}
shader_t* resource_get_shader(resource_t* resource) {
    SASSERT(resource, "Cannot load null resource data");
    SASSERT(resource->type == RESOURCE_TYPE_SHADER, "Cannot load shader from resource type. Resource is not a shader.");
    return shader_loader_get_shader(resource->internal_index);
}
material_t* resource_get_material(resource_t* resource) {
    SASSERT(resource, "Cannot load null resource data");
    SASSERT(resource->type == RESOURCE_TYPE_MATERIAL, "Cannot load material from resource type. Resource is not a material.");
    return material_loader_get_material(resource->internal_index);
}
entity_t resource_instance_model(resource_t* resource, u32 material_count, material_t** materials) {
    SASSERT(resource, "Cannot load null resource data");
    SASSERT(resource->type == RESOURCE_TYPE_MODEL, "Cannot load model from resource type. Resource is not a model, got type %d", resource->type);
    return model_loader_instance_model(resource->internal_index, material_count, materials);
}
