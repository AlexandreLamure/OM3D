#include "structs.glsl"

#ifndef PI
#define PI 3.14159265359
#endif

float sqr(float x) {
    return x * x;
}

float saturate(float x) {
    return min(1.0, max(0.0, x));
}

vec2 saturate(vec2 x) {
    return min(vec2(1.0), max(vec2(0.0), x));
}

vec3 saturate(vec3 x) {
    return min(vec3(1.0), max(vec3(0.0), x));
}

vec4 saturate(vec4 x) {
    return min(vec4(1.0), max(vec4(0.0), x));
}

float luminance(vec3 rgb) {
    return dot(rgb, vec3(0.2126, 0.7152, 0.0722));
}

float attenuation(float distance, float radius) {
    const float x = min(distance, radius);
    return sqr(1.0 - sqr(sqr(x / radius))) / (sqr(x) + 1.0);
}

float attenuation(float distance, float radius, float falloff) {
    return attenuation(distance * falloff, radius * falloff);
}

float sRGB_to_linear(float x) {
    if(x <= 0.04045) {
        return x / 12.92;
    }
    return pow((x + 0.055) / 1.055, 2.4);
}

float linear_to_sRGB(float x) {
    if(x <= 0.0031308) {
        return x * 12.92;
    }
    return 1.055 * pow(x, 1.0 / 2.4) - 0.055;
}

vec3 sRGB_to_linear(vec3 v) {
    return vec3(sRGB_to_linear(v.r), sRGB_to_linear(v.g), sRGB_to_linear(v.b));
}

vec3 linear_to_sRGB(vec3 v) {
    return vec3(linear_to_sRGB(v.r), linear_to_sRGB(v.g), linear_to_sRGB(v.b));
}

vec3 unpack_normal_map(vec2 normal) {
    normal = normal * 2.0 - vec2(1.0);
    return vec3(normal, 1.0 - sqrt(dot(normal, normal)));
}

vec2 to_equirec(vec3 v) {
    return -vec2(atan(-v.y, v.x), asin(v.z)) * vec2(0.1591, 0.3183) + 0.5;
}

vec2 hammersley(uint i, uint N) {
    uint bits = (i << 16u) | (i >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return vec2(float(i) / float(N), float(bits) * 2.3283064365386963e-10);
}

vec3 importance_sample_ggx(vec2 Xi, vec3 N, float roughness) {
    float a = roughness * roughness;

    const float phi = 2.0 * PI * Xi.x;
    const float cos_theta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    const float sin_theta = sqrt(1.0 - cos_theta * cos_theta);
    const vec3 H = vec3(cos(phi) * sin_theta, sin(phi) * sin_theta, cos_theta);
    const vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    const vec3 tangent   = normalize(cross(up, N));
    const vec3 bitangent = cross(N, tangent);

    return normalize(tangent * H.x + bitangent * H.y + N * H.z);
}

float geometry_schlick_ggx(float NdotV, float roughness) {
    const float k = (roughness * roughness) / 2.0;
    const float denom = NdotV * (1.0 - k) + k;

    return NdotV / denom;
}

float geometry_smith(vec3 N, vec3 V, vec3 L, float roughness) {
    const float ggx2 = geometry_schlick_ggx(max(dot(N, V), 0.0), roughness);
    const float ggx1 = geometry_schlick_ggx(max(dot(N, L), 0.0), roughness);

    return ggx1 * ggx2;
}
