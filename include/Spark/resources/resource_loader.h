#pragma once 

#include "Spark/memory/linear_allocator.h"
#include "Spark/resources/resource_types.h"

void resource_loader_initialize(linear_allocator_t* allocator);
void resource_loader_shutdown();

resource_t resource_loader_get_resource(const char* path, b8 auto_delete);
void resource_loader_destroy_resource(resource_t* resource);

