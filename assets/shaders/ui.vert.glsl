#version 450

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec2 tex_coord;
layout(location = 1) out vec4 color;

layout(set = 0, binding = 0) readonly uniform world_data { mat4 matrix; } world;
layout(set = 0, binding = 1) readonly buffer instance_data { mat4 matrix; } instance;

void main() {
    vec2 pos = position;
    tex_coord = uv;
    color = vec4(1);
    gl_Position = world.matrix * instance.matrix * vec4(pos, 1.0f, 1.0f);
}
