#include "Camera.h"

Camera::Camera() {
    _projection = glm::infinitePerspective(to_rad(60.0f), 640.0f / 480.0f, 0.001f);
    _view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    update();
}

void Camera::set_view(const glm::mat4& matrix) {
    _view = matrix;
    update();
}

void Camera::set_proj(const glm::mat4& matrix) {
    _projection = matrix;
    update();
}

void Camera::update() {
    _view_proj = _projection * _view;
}

const glm::mat4& Camera::projection_matrix() const {
    return _projection;
}

const glm::mat4& Camera::view_matrix() const {
    return _view;
}

const glm::mat4& Camera::view_proj_matrix() const {
    return _view_proj;
}
