struct CameraData {
    mat4 view_proj;
};

struct FrameData {
    CameraData camera;
    uint point_light_count;
};

struct PointLight {
    vec3 position;
    float radius;
    vec3 color;
    float padding_1;
};

