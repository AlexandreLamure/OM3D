#version 450

#include "utils.glsl"

layout(location = 0) out vec4 out_color;
layout(location = 1) in vec2 in_uv;

layout(binding = 0) uniform sampler2D in_albedo;
layout(binding = 1) uniform sampler2D in_normal;
layout(binding = 2) uniform sampler2D in_depth;
uniform uint light_id;
uniform uint wireframe;

layout(binding = 0) uniform Data {
    FrameData frame;
};

layout(binding = 1) buffer PointLights {
    PointLight point_lights[];
};

vec3 unproject(vec2 uv, float depth, mat4 inv_viewproj) {
    const vec3 ndc = vec3(uv * 2.0 - vec2(1.0), depth);
    const vec4 p = inv_viewproj * vec4(ndc, 1.0);
    return p.xyz / p.w;
}

void main() {
    if (wireframe != 0)
    {
        out_color = vec4(point_lights[light_id].color, 1.0);
        return;
    }

    ivec2 dims = textureSize(in_albedo, 0);

    const ivec2 coord = ivec2(gl_FragCoord.xy);
    const vec3 albedo = texelFetch(in_albedo, coord, 0).rgb;
    vec3 normal = texelFetch(in_normal, coord, 0).rgb;
    normal = 2.0 * normal - 1.0;

    const float depth = texelFetch(in_depth, coord, 0).x;

    vec3 position = unproject(gl_FragCoord.xy / dims, depth, inverse(frame.camera.view_proj));

    vec3 acc = vec3(0.0);

    PointLight light = point_lights[light_id];
    const vec3 to_light = (light.position - position);
    const float dist = length(to_light);
    const float radius = light.radius;

    if (dist >= radius)
    {
        out_color = vec4(0.0);
        return;
    }

    // out_color = vec4(light.position, 1.0);
    // return;

    const vec3 light_vec = to_light / dist;
    const float NoL = dot(light_vec, normal);
    const float att = attenuation(dist, radius);

    if (!(NoL <= 0.0 || att <= 0.0))
        acc += light.color * (NoL * att);

    out_color = vec4(albedo * acc, 1.0);
}
