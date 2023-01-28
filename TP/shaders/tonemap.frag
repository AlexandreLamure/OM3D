#version 450

#include "utils.glsl"

layout(location = 0) out vec4 out_color;

layout(location = 0) in vec3 in_normal;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_color;
layout(location = 3) in vec3 in_position;
layout(location = 4) in vec3 in_tangent;
layout(location = 5) in vec3 in_bitangent;

layout(binding = 0) uniform sampler2D in_hdr;

uniform float exposure = 1.0;



float reinhard(float hdr) {
    return hdr / (hdr + 1.0);
}

vec3 reinhard(vec3 x) {
    return vec3(reinhard(x.x), reinhard(x.y), reinhard(x.z));
}

void main() {
    const ivec2 coord = ivec2(gl_FragCoord.xy);

    const vec3 hdr = texelFetch(in_hdr, coord, 0).rgb * exposure;
    const vec3 tone_mapped = reinhard(hdr);

    out_color = vec4(hdr, 1.0);
}


