#version 450

#include "utils.glsl"

layout(location = 0) out vec4 out_color;

layout(location = 0) in vec2 in_uv;

layout(binding = 0) uniform sampler2D in_hdr;

uniform float exposure = 1.0;

vec3 aces(vec3 x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return saturate((x*(a*x+b))/(x*(c*x+d)+e));
}

float reinhard(float hdr) {
    return hdr / (hdr + 1.0);
}

vec3 reinhard(vec3 x) {
    return vec3(reinhard(x.x), reinhard(x.y), reinhard(x.z));
}

void main() {
    const ivec2 coord = ivec2(gl_FragCoord.xy);

    const vec3 hdr = texelFetch(in_hdr, coord, 0).rgb * exposure;
    const vec3 tone_mapped = aces(hdr);

    out_color = vec4(hdr, 1.0);
}


