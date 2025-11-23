#version 450

layout(location = 0) out vec4 out_color;

layout(location = 0) in vec2 in_uv;

layout(binding = 0) uniform sampler2D in_tex;

void main() {
    out_color = texture(in_tex, in_uv);
}


