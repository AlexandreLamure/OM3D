#include "utils.glsl"


float d_ggx(vec3 N, vec3 H, float roughness) {
    const float a = roughness * roughness;
    const float a2 = a*a;
    const float NdotH = max(dot(N, H), 0.0);
    const float NdotH2 = NdotH * NdotH;

    const float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    return a2 / (PI * denom * denom);
}

float g_schlick_ggx(float NdotV, float k) {
    return NdotV / (NdotV * (1.0 - k) + k);
}

float g_smith(vec3 N, vec3 V, vec3 L, float roughness) {
    const float r = (roughness + 1.0);
    const float k = (r * r) / 8.0;
    const float ggx2 = g_schlick_ggx(max(dot(N, V), 0.0), k);
    const float ggx1 = g_schlick_ggx(max(dot(N, L), 0.0), k);
    return ggx1 * ggx2;
}

vec3 f_schlick(float cos_theta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cos_theta, 0.0, 1.0), 5.0);
}


vec3 eval_brdf(vec3 N, vec3 V, vec3 L, vec3 albedo, float metallic, float roughness) {
    const float NdotL = dot(N, L);

    if(NdotL <= 0.0) {
        return vec3(0.0);
    }

    const vec3 H = normalize(V + L);

    const float D = d_ggx(N, H, roughness);
    const float G = g_smith(N, V, L, roughness);

    const vec3 F0 = mix(vec3(0.04), albedo, metallic);
    const vec3 F = f_schlick(max(dot(H, V), 0.0), F0);

    const float specular = (D * G) / (4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001);
    const vec3 diffuse = albedo * INV_PI * (1.0 - metallic);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;

    return (diffuse * kD + specular * kS) * NdotL;
}
