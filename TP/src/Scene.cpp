#include "Scene.h"

#include <TypedBuffer.h>

Scene::Scene() {
}

void Scene::add_object(SceneObject obj) {
    _objects.emplace_back(std::move(obj));
}

void Scene::render(const Camera& camera) const {
    TypedBuffer<glm::mat4> buffer(nullptr, 1);
    buffer.map(MappingType::WriteOnly)[0] = camera.view_proj_matrix();

    buffer.bind(BufferUsage::Uniform, 0);

    for(const SceneObject& obj : _objects) {
        obj.render();
    }
}
