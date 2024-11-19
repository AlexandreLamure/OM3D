#include "StaticMesh.h"

#include <functional>
#include <glad/gl.h>
#include <numeric>

#include "Camera.h"
#include "glm/ext/vector_float3.hpp"
#include "glm/geometric.hpp"

namespace OM3D
{

    extern bool audit_bindings_before_draw;

    StaticMesh::StaticMesh(const MeshData& data)
        : _vertex_buffer(data.vertices)
        , _index_buffer(data.indices)
    {
        // create a bounding sphere
        glm::vec3 center = { 0.f, 0.f, 0.f };
        for (const auto& vertex : data.vertices)
        {
            center += vertex.position;
        }
        center /= data.vertices.size();

        Vertex far_point =
            std::reduce(data.vertices.cbegin(), data.vertices.cend(),
                        *data.vertices.begin(),
                        [&center](const Vertex& v, const Vertex& curr) {
                            float dist1 = glm::length(v.position - center);
                            float dist2 = glm::length(curr.position - center);
                            if (dist1 > dist2)
                                return v;
                            return curr;
                        });

        _bounding_sphere = { center, far_point.position };
    }

    static bool in_plane(const glm::vec3& n, const glm::vec3& p,
                         const glm::vec3 center, float radius)
    {
        glm::vec3 v = center + glm::normalize(n) * radius;
        glm::vec3 x = v - p;
        return glm::dot(x, n) >= 0;
    }

    void StaticMesh::draw(const glm::vec3& camera, const glm::mat4& transform,
                          const Frustum& f) const
    {
        //  Updating the bounding sphere with the transformation applied to the
        //  object
        glm::vec3 updated_center =
            transform * glm::vec4(_bounding_sphere.center, 1.0);
        glm::vec3 updated_far_point =
            transform * glm::vec4(_bounding_sphere.far_point, 1.0);
        float radius = length(updated_center - updated_far_point);

        // Checking if the object is visible to the camera
        bool to_draw = in_plane(f._left_normal, camera, updated_center, radius);
        to_draw &= in_plane(f._top_normal, camera, updated_center, radius);
        to_draw &= in_plane(f._right_normal, camera, updated_center, radius);
        to_draw &= in_plane(f._bottom_normal, camera, updated_center, radius);
        to_draw &= in_plane(f._near_normal, camera, updated_center, radius);
        if (!to_draw)
            return;

        _vertex_buffer.bind(BufferUsage::Attribute);
        _index_buffer.bind(BufferUsage::Index);

        // Vertex position
        glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex), nullptr);
        // Vertex normal
        glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(Vertex),
                              reinterpret_cast<void*>(3 * sizeof(float)));
        // Vertex uv
        glVertexAttribPointer(2, 2, GL_FLOAT, false, sizeof(Vertex),
                              reinterpret_cast<void*>(6 * sizeof(float)));
        // Tangent / bitangent sign
        glVertexAttribPointer(3, 4, GL_FLOAT, false, sizeof(Vertex),
                              reinterpret_cast<void*>(8 * sizeof(float)));
        // Vertex color
        glVertexAttribPointer(4, 3, GL_FLOAT, false, sizeof(Vertex),
                              reinterpret_cast<void*>(12 * sizeof(float)));

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glEnableVertexAttribArray(3);
        glEnableVertexAttribArray(4);

        if (audit_bindings_before_draw)
        {
            audit_bindings();
        }

        glDrawElements(GL_TRIANGLES, int(_index_buffer.element_count()),
                       GL_UNSIGNED_INT, nullptr);
    }

} // namespace OM3D
