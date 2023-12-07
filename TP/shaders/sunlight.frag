#version 450

layout(location = 0) uniform sampler2D albedo;
layout(location = 1) uniform sampler2D normal;
layout(location = 2) uniform sampler2D depth;

out vec4 outColor;

void main() {
    const ivec2 coord = ivec2(gl_FragCoord.xy);

    outColor = vec4(texelFetch(albedo, coord, 0).rgb, 1.0);
}