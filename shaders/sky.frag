#version 450

#include "utils.glsl"

layout(location = 0) out vec4 out_color;

layout(location = 0) in vec2 in_uv;

layout(binding = 0) uniform Data {
    FrameData frame;
};

layout(binding = 4) uniform samplerCube in_envmap;

uniform float intensity;

void main() {
    const vec3 view_dir = normalize(unproject(in_uv, 0.001, frame.camera.inv_view_proj) - frame.camera.position);
    out_color = texture(in_envmap, view_dir) * intensity;
}


