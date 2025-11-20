#include "BoundingSphere.h"

bool BoundingSphere::collideFrustum(const OM3D::Camera camera,
                                    const OM3D::Frustum frustum,
                                    const glm::vec3 &scale,
                                    const glm::vec3 &translate) const
{
    const glm::vec3 v = (_center * scale) - camera.position() + translate;
    const float r = _radius * std::max(scale[0], std::max(scale[1], scale[2])) * frustum._culling_bounding_sphere_coeff;

    return (glm::dot(v, glm::normalize(frustum._bottom_normal)) >= -r)
        && (glm::dot(v, glm::normalize(frustum._left_normal)) >= -r)
        && (glm::dot(v, glm::normalize(frustum._right_normal)) >= -r)
        && (glm::dot(v, glm::normalize(frustum._near_normal)) >= -r)
        && (glm::dot(v, glm::normalize(frustum._top_normal)) >= -r);
}
