#include "SceneObject.h"

#include <glm/gtc/matrix_transform.hpp>

SceneObject::SceneObject(std::shared_ptr<StaticMesh> mesh, std::shared_ptr<Program> shader) :
    _mesh(std::move(mesh)),
    _shader(std::move(shader)) {
}

void SceneObject::render() const {
    if(!_shader || !_mesh) {
        return;
    }

   _shader->set_uniform(HASH("model"), transform());
   _shader->set_uniform(HASH("red"), float(std::sin(program_time()) * 0.5f + 0.5f));
   _shader->set_uniform(HASH("green"), 0.5f);
   _shader->set_uniform(HASH("blue"), 1.0f);
   _shader->bind();
    _mesh->draw();
}

const glm::mat4& SceneObject::transform() const {
    return _transform;
}
