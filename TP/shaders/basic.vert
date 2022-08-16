#version 450

#include "utils.glsl"

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;
layout(location = 3) in vec3 in_color;

layout(location = 0) out vec3 out_normal;
layout(location = 1) out vec2 out_uv;
layout(location = 2) out vec3 out_color;
layout(location = 3) out vec3 out_position;

layout(binding = 0) uniform Data {
    FrameData frame;
};

uniform mat4 model;

void main() {
    const vec4 position = model * vec4(in_pos, 1.0);

    out_normal = mat3(model) * in_normal;
    out_uv = in_uv;
    out_color = in_color;
    out_position = position.xyz;

    gl_Position = frame.camera.view_proj * position;
}

