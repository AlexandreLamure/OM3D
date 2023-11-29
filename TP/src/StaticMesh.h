#ifndef STATICMESH_H
#define STATICMESH_H

#include <graphics.h>
#include <TypedBuffer.h>
#include <Vertex.h>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include <vector>

namespace OM3D {

struct SphericalBoundingBox {
    glm::vec3 center;
    float radius;
};

struct MeshData {
    std::vector<Vertex> vertices;
    std::vector<u32> indices;
};

class StaticMesh : NonCopyable {

    public:
        StaticMesh() = default;
        StaticMesh(StaticMesh&&) = default;
        StaticMesh& operator=(StaticMesh&&) = default;

        StaticMesh(const MeshData& data);

        // Getter for bounding box
        const SphericalBoundingBox& bounding_box() const;

        // equal operator
        bool operator==(const StaticMesh& other) const {
            return _vertex_buffer == other._vertex_buffer && _index_buffer == other._index_buffer;
        }

        void draw() const;

    private:
        TypedBuffer<Vertex> _vertex_buffer;
        TypedBuffer<u32> _index_buffer;

        SphericalBoundingBox _bounding_box;
};

}

#endif // STATICMESH_H
