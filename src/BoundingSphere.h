#pragma once

#include "Camera.h"
#include "glm/vec3.hpp"

class BoundingSphere {
    glm::vec3 _center;
    float _radius;

public:
    BoundingSphere() = default;
    BoundingSphere(const glm::vec3 center, const float radius) : _center(center), _radius(radius) {
    };


    bool collideFrustum(OM3D::Camera, OM3D::Frustum) const;
};
