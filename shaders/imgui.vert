#version 450

layout(location = 0) in vec2 in_pos;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec4 in_color;

layout(location = 0) out vec2 out_uv;
layout(location = 1) out vec4 out_color;

uniform vec2 viewport_size;

void main() {
    out_uv = in_uv;
    out_color = in_color / 255.0;


    const mat4 proj = mat4(2.0 / viewport_size.x, 0.0, 0.0, 0.0,
                           0.0, 2.0 / -viewport_size.y, 0.0, 0.0,
                           0.0, 0.0, -1.0, 0.0,
                           -1.0, 1.0, 0.0, 1.0);

    gl_Position = proj * vec4(in_pos, 0.0, 1.0);
}

