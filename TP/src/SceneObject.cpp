#include "SceneObject.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>

#include <iostream>
#include <ostream>

namespace OM3D {

SceneObject::SceneObject(std::shared_ptr<StaticMesh> mesh, std::shared_ptr<Material> material) :
    _mesh(std::move(mesh)),
    _material(std::move(material)) {
}

void SceneObject::setup() const {
    if(!_material || !_mesh) {
        return;
    }

    _material->set_uniform(HASH("model"), transform());
    _material->set_backface_culling(true);
    _material->bind();

    _mesh->setup();
    // Disable backface culling for the rest of the objects
    glDisable(GL_CULL_FACE);
}

void SceneObject::render() const {
    if(!_material || !_mesh) {
        return;
    }

    _material->set_uniform(HASH("model"), transform());
    _material->set_backface_culling(true);
    _material->bind();
    _mesh->draw();

    // Disable backface culling for the rest of the objects
    glDisable(GL_CULL_FACE);
}

void SceneObject::set_transform(const glm::mat4& tr) {
    _transform = tr;
}

const glm::mat4& SceneObject::transform() const {
    return _transform;
}

bool SceneObject::check_frustum(const Camera camera) const {
    auto frustum = camera.build_frustum();
    auto bounding_sphere = bounding_box();
    auto camToSphere = bounding_sphere.center - camera.position();

    float distToNear = glm::dot(camToSphere, frustum._near_normal) + bounding_sphere.radius;
    float distToTop = glm::dot(camToSphere, frustum._top_normal) + bounding_sphere.radius;
    float distToBottom = glm::dot(camToSphere, frustum._bottom_normal) + bounding_sphere.radius;
    float distToRight = glm::dot(camToSphere, frustum._right_normal) + bounding_sphere.radius;
    float distToLeft = glm::dot(camToSphere, frustum._left_normal) + bounding_sphere.radius;

    // Check if the sphere is completely outside any of the frustum planes
    if(distToNear < 0.0f || distToTop < 0.0f || distToBottom < 0.0f || distToRight < 0.0f || distToLeft < 0.0f) {
        return false;
    }

    // Print the distance to the near plane
    //std::cout << "Dist to planes: " << distToRight << ", " << distToLeft << ", " << distToTop << ", " << distToBottom << ", " << distToNear << std::endl;
    return true;  // Bounding sphere is at least partially inside the frustum
}

SphericalBoundingBox SceneObject::bounding_box() const {
    return _mesh->bounding_box();
}

}
