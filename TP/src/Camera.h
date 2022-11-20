#ifndef CAMERA_H
#define CAMERA_H

#include <glm/gtc/matrix_transform.hpp>

#include <utils.h>

namespace OM3D {

struct Frustum {
    glm::vec3 _near_normal;
    // No far plane (zFar is +inf)
    glm::vec3 _top_normal;
    glm::vec3 _bottom_normal;
    glm::vec3 _right_normal;
    glm::vec3 _left_normal;
};


class Camera {
    public:
        Camera();

        void set_view(const glm::mat4& matrix);
        void set_proj(const glm::mat4& matrix);

        glm::vec3 position() const;
        glm::vec3 forward() const;
        glm::vec3 right() const;
        glm::vec3 up() const;

        const glm::mat4& projection_matrix() const;
        const glm::mat4& view_matrix() const;
        const glm::mat4& view_proj_matrix() const;

        Frustum build_frustum() const;

    private:
        void update();
        glm::mat4 build_projection(float zNear);

        glm::mat4 _projection;
        glm::mat4 _view;
        glm::mat4 _view_proj;

        float _fov_y;
        float _aspect_ratio;
};

}

#endif // CAMERA_H
