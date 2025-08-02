#pragma once

#include "Spark/ecs/ecs.h"

void transform_system_initialize(struct ecs_world* world);
void camera_systems_initialize(struct ecs_world* world);

void render_system_initialize(struct ecs_world* world);
void render_system_shutdown();
