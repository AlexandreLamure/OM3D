#ifndef STATICMESH_H
#define STATICMESH_H

#include <graphics.h>
#include <TypedBuffer.h>
#include <Vertex.h>

#include <vector>

struct MeshData {
    std::vector<Vertex> vertices;
    std::vector<u32> indexes;
};

class StaticMesh : NonCopyable {

    public:
        StaticMesh() = default;
        StaticMesh(StaticMesh&&) = default;
        StaticMesh& operator=(StaticMesh&&) = default;

        StaticMesh(const MeshData& data);

    private:
        TypedBuffer<Vertex> _vertex_buffer;
        TypedBuffer<u32> _index_buffer;


};

#endif // STATICMESH_H
