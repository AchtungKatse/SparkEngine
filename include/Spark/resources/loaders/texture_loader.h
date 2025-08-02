#pragma once

#include "Spark/memory/linear_allocator.h"
#include "Spark/renderer/texture.h"
#include "Spark/resources/resource_types.h"

void texture_loader_initialzie(linear_allocator_t* allocator);
void texture_loader_shutdown();

resource_t texture_loader_load_text_resouce(const char* text, u32 length, b8 auto_delete);
texture_t* texture_loader_get_texture(u32 index);
texture_t load_texture_from_config(texture_config_t* config);
