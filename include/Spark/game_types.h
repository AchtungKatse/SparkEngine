#pragma once

struct application_state;

#include "Spark/core/application.h"

typedef struct game {
    application_config_t config;
    void* application_state;

    b8 (*initialize)    (struct game* game_inst);
    b8 (*shutdown)      (struct game* game_inst);
    b8 (*update)        (struct game* game_inst, f32 delta_time);
    b8 (*render)        (struct game* game_inst, f32 delta_time);
    b8 (*on_resize)     (struct game* game_inst, u32 width, u32 height);
    void* state;
} game_t;
