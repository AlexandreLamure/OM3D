#include "Scene.h"

#include <TypedBuffer.h>

#include <shader_structs.h>

Scene::Scene() {
}

void Scene::add_object(SceneObject obj) {
    _objects.emplace_back(std::move(obj));
}

void Scene::add_object(PointLight obj) {
    _point_lights.emplace_back(std::move(obj));
}

void Scene::render(const Camera& camera) const {
    TypedBuffer<shader::CameraData> camera_buffer(nullptr, 1);
    camera_buffer.map(MappingType::WriteOnly)[0].view_proj = camera.view_proj_matrix();
    camera_buffer.bind(BufferUsage::Uniform, 0);
    
    ByteBuffer light_buffer(nullptr, 4 * sizeof(u32) + _point_lights.size() * sizeof(shader::PointLight));
    {
        auto light_buffer_mapping = light_buffer.map_bytes(MappingType::WriteOnly);
        *reinterpret_cast<u32*>(light_buffer_mapping.data()) = u32(_point_lights.size());
        shader::PointLight* data = reinterpret_cast<shader::PointLight*>(light_buffer_mapping.data() + 4 * sizeof(u32));
        for(size_t i = 0; i != _point_lights.size(); ++i) {
            const auto& light = _point_lights[i];
            data[i] = {
                light.position(),
                0.0f,
                light.color(),
                0.0f
            };
        }
    }
    light_buffer.bind(BufferUsage::Storage, 1);
        
    for(const SceneObject& obj : _objects) {
        obj.render();
    }
}
