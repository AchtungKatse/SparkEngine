#pragma once

struct game;

#include "Spark/defines.h"

typedef struct {
    s16 start_pos_x;
    s16 start_pos_y;
    s16 start_width;
    s16 start_height;

    const char* name;
} application_config_t;


SAPI b8 application_create(struct game* game_inst);
SAPI b8 application_run();

SAPI void application_get_framebuffer_size(u32* width, u32* height);
