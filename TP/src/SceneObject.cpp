#include "SceneObject.h"

#include <glm/gtc/matrix_transform.hpp>

namespace OM3D {

SceneObject::SceneObject(std::shared_ptr<StaticMesh> mesh, std::shared_ptr<Material> material) :
    _mesh(std::move(mesh)),
    _material(std::move(material)) {
}

void SceneObject::render() const {
    if(!_material || !_mesh) {
        return;
    }

   _material->set_uniform(HASH("model"), transform());
   _material->bind();
    _mesh->draw();
}

const glm::mat4& SceneObject::transform() const {
    return _transform;
}

}
