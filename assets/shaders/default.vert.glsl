#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 uv;

layout(location = 0) out vec3 normal;
layout(location = 1) out vec2 tex_coord;

layout(set = 0, binding = 0) readonly uniform world_data { mat4 view_matrix; } world;
layout(set = 0, binding = 1) readonly buffer instance_data { mat4 matrix; } instance;

void main() {
    vec3 pos = position;
    tex_coord = uv;
    normal = in_normal;
    gl_Position = world.view_matrix * instance.matrix * vec4(pos, 1.0f);
}

