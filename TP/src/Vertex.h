#ifndef VERTEX_H
#define VERTEX_H

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace OM3D {

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
    glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f); // to avoid completly black meshes if no color is present
};

}

#endif // VERTEX_H
