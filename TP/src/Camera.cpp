#include "Camera.h"

namespace OM3D {

static glm::vec3 extract_position(const glm::mat4& view) {
    glm::vec3 pos = {};
    for(u32 i = 0; i != 3; ++i) {
        pos -= glm::vec3(view[0][i], view[1][i], view[2][i]) * view[3][i];
    }
    return pos;
}

static glm::vec3 extract_forward(const glm::mat4& view) {
    return -glm::normalize(glm::vec3(view[0][2], view[1][2], view[2][2]));
}

static glm::vec3 extract_right(const glm::mat4& view) {
    return glm::normalize(glm::vec3(view[0][0], view[1][0], view[2][0]));
}

static glm::vec3 extract_up(const glm::mat4& view) {
    return glm::normalize(glm::vec3(view[0][1], view[1][1], view[2][1]));
}

Camera::Camera() {
    _projection = glm::infinitePerspective(to_rad(60.0f), 16.0f / 9.0f, 0.001f);
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

glm::vec3 Camera::position() const {
    return extract_position(_view);
}

glm::vec3 Camera::forward() const {
    return extract_forward(_view);
}

glm::vec3 Camera::right() const {
    return extract_right(_view);
}

glm::vec3 Camera::up() const {
    return extract_up(_view);
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

void Camera::update() {
    _view_proj = _projection * _view;
}

}
