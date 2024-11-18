#ifndef STATICMESH_H
#define STATICMESH_H

#include <TypedBuffer.h>
#include <Vertex.h>
#include <graphics.h>
#include <vector>

#include "Camera.h"

namespace OM3D
{

    struct Sphere
    {
        glm::vec3 center;
        float radius;
    };

    struct MeshData
    {
        std::vector<Vertex> vertices;
        std::vector<u32> indices;
    };

    class StaticMesh : NonCopyable
    {
    public:
        StaticMesh() = default;
        StaticMesh(StaticMesh&&) = default;
        StaticMesh& operator=(StaticMesh&&) = default;

        StaticMesh(const MeshData& data);

        void draw(const glm::vec3& pos, const Frustum& frustum) const;

    private:
        TypedBuffer<Vertex> _vertex_buffer;
        TypedBuffer<u32> _index_buffer;
        Sphere _bounding_sphere;
    };

} // namespace OM3D

#endif // STATICMESH_H
