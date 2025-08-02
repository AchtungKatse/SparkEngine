#version 450

layout(location = 0) in vec3 normal;
layout(location = 1) in vec2 tex_coord;

layout(location = 0) out vec4 out_color;

layout(set = 1, binding = 0) uniform sampler2D main_texture;


void main() {
    out_color = texture(main_texture, tex_coord);
    // out_color = vec4(1,1,1,1);
    vec3 ambient = vec3(.1);
    out_color.xyz += ambient;
    out_color.xyz *= (1.0 + dot(normal, vec3(.707, .707, .707))) / 2;
}
