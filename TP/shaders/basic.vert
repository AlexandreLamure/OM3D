#version 450
layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec3 in_color;

layout(location = 0) out vec3 out_color;
layout(location = 1) out vec3 out_normal;

uniform mat4 view_proj;
uniform mat4 model;

void main() {
    out_color = in_color;
    out_normal = mat3(model) * in_normal;
    gl_Position = view_proj * model * vec4(in_pos, 1.0);
}

