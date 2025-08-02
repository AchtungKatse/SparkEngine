#pragma once

#include "Spark/ecs/entity.h"
#define UI_TEXT_MAX_LENGTH 2048

typedef struct ui_text {
    const char* text;
} ui_text_t;

entity_t ui_text_create(ecs_world_t* world, const char* text);
entity_t ui_text_update(const char* text);
