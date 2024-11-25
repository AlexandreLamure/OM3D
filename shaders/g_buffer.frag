#version 450

#include "utils.glsl"

// fragment shader of filling g buffer pass

layout(location = 0) out vec4 out_albedo;
layout(location = 1) out vec4 out_normal;

layout(location = 0) in vec3 in_normal;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_color;
layout(location = 3) in vec3 in_position;
layout(location = 4) in vec3 in_tangent;
layout(location = 5) in vec3 in_bitangent;

layout(binding = 0) uniform sampler2D in_texture;
layout(binding = 1) uniform sampler2D in_normal_texture;

layout(binding = 0) uniform Data {
    FrameData frame;
};

const vec3 ambient = vec3(0.0);

vec3 remap(vec3 n)
{
    return (n + 1.f) / 2.f;
}

void main() {
    #ifdef NORMAL_MAPPED
    const vec3 normal_map = unpack_normal_map(texture(in_normal_texture, in_uv).xy);
    const vec3 normal = normal_map.x * in_tangent +
            normal_map.y * in_bitangent +
            normal_map.z * in_normal;
    #else
    const vec3 normal = in_normal;
    #endif
    out_normal = vec4(remap(normal), 1.f);

    out_albedo = vec4(in_color, 1.0);

    #ifdef TEXTURED
    out_albedo *= texture(in_texture, in_uv);
    #endif
}
