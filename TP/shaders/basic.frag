#version 450

layout(location = 0) out vec4 out_color;

layout(location = 0) in vec3 in_normal;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_color;


layout(binding = 0) uniform sampler2D in_texture;

void main() {
    const float l = max(0.0, dot(in_normal, normalize(vec3(0.5, 1.0, 2.0)))) * 0.5 + 0.5;
    out_color = vec4(in_color * l, 1.0) * texture(in_texture, in_uv);
}

