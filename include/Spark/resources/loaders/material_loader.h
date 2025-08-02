#pragma once

#include "Spark/memory/linear_allocator.h"
#include "Spark/renderer/material.h"
#include "Spark/resources/resource_types.h"

void material_loader_initialzie(linear_allocator_t* allocator);
void material_loader_shutdown();

resource_t material_loader_load_text_resouce(const char* text, u32 length, b8 auto_delete);
material_t* material_loader_get_material(u32 index);
material_t* material_loader_create_from_config(material_config_t* config);
