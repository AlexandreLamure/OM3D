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

        float radius = std::transform_reduce(
            data.vertices.cbegin(), data.vertices.cend(), 0.f,
            [](float a, float b) { return std::max(a, b); },
            [&center](const Vertex& v) {
                return glm::length(v.position - center);
            });

        _bounding_sphere = { center, radius };
    }

    // static bool in_plane(const glm::vec3& n, const glm::vec3& p,
    //                      const Sphere& s)
    // {
    //     glm::vec3 v = s.center + glm::normalize(n) * s.radius;
    //     glm::vec3 x = v - p;
    //     return glm::dot(x, n) >= 0;
    // }

    void StaticMesh::draw(const glm::vec3& camera, const Frustum& f) const
    {
        (void)f;
        (void)camera;
        // Checking the frustum part
        // bool to_draw = in_plane(f._left_normal, camera, _bounding_sphere);
        // to_draw &= in_plane(f._top_normal, camera, _bounding_sphere);
        // to_draw &= in_plane(f._right_normal, camera, _bounding_sphere);
        // to_draw &= in_plane(f._bottom_normal, camera, _bounding_sphere);
        // to_draw &= in_plane(f._near_normal, camera, _bounding_sphere);
        // if (!to_draw)
        //     return;

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
