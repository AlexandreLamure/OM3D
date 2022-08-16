#include "structs.glsl"

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
