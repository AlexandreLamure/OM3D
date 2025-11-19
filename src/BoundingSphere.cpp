#include "BoundingSphere.h"

bool BoundingSphere::collideFrustum(const OM3D::Camera camera,
                                    const OM3D::Frustum frustum) const
{
    const glm::vec3 v = _center - camera.position();
    const float r = _radius;

    return (glm::dot(v, glm::normalize(frustum._bottom_normal)) >= -r)
        && (glm::dot(v, glm::normalize(frustum._left_normal)) >= -r)
        && (glm::dot(v, glm::normalize(frustum._right_normal)) >= -r)
        && (glm::dot(v, glm::normalize(frustum._near_normal)) >= -r)
        && (glm::dot(v, glm::normalize(frustum._top_normal)) >= -r);
}
