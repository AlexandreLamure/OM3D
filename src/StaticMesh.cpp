#include "StaticMesh.h"

#include <glad/gl.h>
#include <glm/geometric.hpp>
#include <iostream>

#include "BoundingSphere.h"

namespace OM3D
{

    extern bool audit_bindings_before_draw;

    StaticMesh::StaticMesh(const MeshData &data)
        : _vertex_buffer(data.vertices)
        , _index_buffer(data.indices)
    {
        glm::vec3 barycenter = { 0, 0, 0 };

        for (unsigned mesh_index = 0; mesh_index < data.vertices.size();
             mesh_index++)
        {
            barycenter += data.vertices[mesh_index].position;
        }
        barycenter /= data.vertices.size();
        float max_squared_distance = 0;
        for (unsigned mesh_index = 0; mesh_index < data.vertices.size();
             mesh_index++)
        {
            glm::vec3 direction =
                data.vertices[mesh_index].position - barycenter;
            max_squared_distance =
                std::max(max_squared_distance, glm::dot(direction, direction));
        }
        // use bounding sphere with barycenter and radius
        // sqrt(max_squared_distance)
        _bounding_sphere =
            BoundingSphere(barycenter, std::sqrt(max_squared_distance));
    }

    void StaticMesh::draw(const Camera &camera, const Frustum &frustum,
                          const glm::vec3 &scale,
                          const glm::vec3 &translation) const
    {
        if (frustum._culling_enabled
            && !_bounding_sphere.collideFrustum(camera, frustum, scale,
                                                translation))
        {
            return;
        }

        _vertex_buffer.bind(BufferUsage::Attribute);
        _index_buffer.bind(BufferUsage::Index);

        // Vertex position
        glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex), nullptr);
        // Vertex normal
        glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(Vertex),
                              reinterpret_cast<void *>(3 * sizeof(float)));
        // Vertex uv
        glVertexAttribPointer(2, 2, GL_FLOAT, false, sizeof(Vertex),
                              reinterpret_cast<void *>(6 * sizeof(float)));
        // Tangent / bitangent sign
        glVertexAttribPointer(3, 4, GL_FLOAT, false, sizeof(Vertex),
                              reinterpret_cast<void *>(8 * sizeof(float)));
        // Vertex color
        glVertexAttribPointer(4, 3, GL_FLOAT, false, sizeof(Vertex),
                              reinterpret_cast<void *>(12 * sizeof(float)));

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
