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

bool SceneObject::check_frustum(const Frustum frustum) const {
    auto bounding_sphere = bounding_box();

    float distToNear = glm::dot(bounding_sphere.center, frustum._near_normal) - bounding_sphere.radius;
    float distToTop = glm::dot(bounding_sphere.center, frustum._top_normal) - bounding_sphere.radius;
    float distToBottom = glm::dot(bounding_sphere.center, frustum._bottom_normal) - bounding_sphere.radius;
    float distToRight = glm::dot(bounding_sphere.center, frustum._right_normal) - bounding_sphere.radius;
    float distToLeft = glm::dot(bounding_sphere.center, frustum._left_normal) - bounding_sphere.radius;

    // Check if the sphere is completely outside any of the frustum planes
    if(distToNear > 0.0f || distToTop > 0.0f || distToBottom > 0.0f || distToRight > 0.0f || distToLeft > 0.0f) {
        return false;
    }

    // Print the distance to the near plane
    std::cout << "Distance to near plane: " << distToNear << std::endl;

    return true;  // Bounding sphere is at least partially inside the frustum
}

SphericalBoundingBox SceneObject::bounding_box() const {
    return _mesh->bounding_box();
}

    StaticMesh &SceneObject::mesh() const {
        return *_mesh;
    }

}
