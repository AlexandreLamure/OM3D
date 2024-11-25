#version 450

#include "utils.glsl"

layout(location = 0) out vec4 out_color;

layout(binding = 0) uniform sampler2D in_albedo;
layout(binding = 1) uniform sampler2D in_normal;
layout(binding = 2) uniform sampler2D in_depth;

uniform uint texture;

void main() {
    const ivec2 coord = ivec2(gl_FragCoord.xy);
    vec3 color = vec3(0.f);

    if (texture < 2)
    {
        if (texture == 0)
            color = texelFetch(in_albedo, coord, 0).rgb;
        else
            color = texelFetch(in_normal, coord, 0).rgb;
    }
    else
    {
        float depth = texelFetch(in_depth, coord, 0).r;
        color = vec3(pow(depth, 0.35));
    }
    out_color = vec4(color, 1.f);
}
