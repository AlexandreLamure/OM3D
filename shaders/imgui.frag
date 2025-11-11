#version 450

#include "utils.glsl"

layout(location = 0) out vec4 out_color;

layout(location = 0) in vec2 in_uv;
layout(location = 1) in vec4 in_color;

layout(binding = 0) uniform sampler2D in_texture;

void main() {
    const vec4 color = in_color * texture(in_texture, in_uv);
    out_color = vec4(sRGB_to_linear(color.rgb), color.a); // Compensate for the conversion made by OpenGL

}

