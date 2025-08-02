#version 450

layout(location = 0) in vec2 tex_coord;
layout(location = 1) in vec4 color;

layout(location = 0) out vec4 out_color;

layout(set = 1, binding = 0) uniform sampler2D main_texture;


void main() {
    out_color = texture(main_texture, tex_coord);
    if (out_color.w <= .5f) {
        discard;
    }
}
