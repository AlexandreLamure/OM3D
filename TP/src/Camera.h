#ifndef CAMERA_H
#define CAMERA_H

#include <glm/gtc/matrix_transform.hpp>

#include <utils.h>

class Camera {
    public:
        Camera();

        void set_view(const glm::mat4& matrix);
        void set_proj(const glm::mat4& matrix);


        const glm::mat4& projection_matrix() const;
        const glm::mat4& view_matrix() const;
        const glm::mat4& view_proj_matrix() const;

    private:
        void update();

        glm::mat4 _projection;
        glm::mat4 _view;

        glm::mat4 _view_proj;
};

#endif // CAMERA_H
