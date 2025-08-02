#pragma once

#include "Spark/renderer/renderer_types.h"

struct platform_state;

/**
 * @brief Creates a renderer backend
 *
 * @param type 
 * @param plat_state 
 * @param out_renderer_backend 
 * @return False if failed to create backend
 */
b8 renderer_backend_create(renderer_backend_type_t type, struct platform_state* plat_state, renderer_backend_t* out_renderer_backend);
void renderer_backend_destroy(renderer_backend_t* renderer_backend);
