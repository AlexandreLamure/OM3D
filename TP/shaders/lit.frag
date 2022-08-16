#version 450

#include "utils.glsl"

layout(location = 0) out vec4 out_color;

layout(location = 0) in vec3 in_normal;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_color;
layout(location = 3) in vec3 in_position;

layout(binding = 0) uniform sampler2D in_texture;

layout(binding = 1) buffer PointLights {
    uint point_light_count;
    PointLight point_lights[];
};

void main() {
    const vec3 color = in_color * texture(in_texture, in_uv).rgb;
    
    vec3 acc = vec3(0.0);
    for(uint i = 0; i != point_light_count; ++i) {
        PointLight light = point_lights[i];
        const vec3 to_light = normalize(light.position - in_position);
        
        const float NoL = max(0.0, dot(to_light, in_normal));
        acc += NoL * color;
    }
    
    out_color = vec4(acc, 1.0);
}

