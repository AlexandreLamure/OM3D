#ifndef VERTEX_H
#define VERTEX_H

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;
};

#endif // VERTEX_H
