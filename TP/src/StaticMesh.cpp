#include "StaticMesh.h"


StaticMesh::StaticMesh(const MeshData& data) :
    _vertex_buffer(data.vertices),
    _index_buffer(data.indexes) {
}
