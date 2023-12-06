#version 450

#include "utils.glsl"

layout(location = 0) out vec4 out_frag;

layout(binding = 0) uniform sampler2D in_color;
layout(binding = 1) uniform sampler2D in_albedo;
layout(binding = 2) uniform sampler2D in_normal;
layout(binding = 3) uniform sampler2D in_depth;

uniform uint g_buffer_mode;

void main() {
    const ivec2 coord = ivec2(gl_FragCoord.xy);
    vec3 frag;
    switch (g_buffer_mode) {
        case 0:
            frag = texelFetch(in_color, coord, 0).rgb;
            break;
        case 1:
            frag = texelFetch(in_albedo, coord, 0).rgb;
            break;
        case 2:
            frag = texelFetch(in_normal, coord, 0).rgb;
            break;
        case 3:
            // compute depth from linear: depth = depth^0.35
            frag = vec3(pow(texelFetch(in_depth, coord, 0).r, 0.25));
            break;
    }
    out_frag = vec4(frag, 1.0);
}


