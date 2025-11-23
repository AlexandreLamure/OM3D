struct CameraData {
    mat4 view_proj;
    mat4 inv_view_proj;
    vec3 position;
    float padding;
};

struct FrameData {
    CameraData camera;

    vec3 sun_dir;
    uint point_light_count;

    vec3 sun_color;
    float ibl_intensity;
};

struct PointLight {
    vec3 position;
    float radius;
    vec3 color;
    float padding;
};

