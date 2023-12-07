#include "Scene.h"

#include <TypedBuffer.h>

#include <algorithm>
#include <shader_structs.h>

namespace OM3D {

Scene::Scene() {
}

void Scene::add_object(SceneObject obj) {
    if (std::find(_materials.begin(), _materials.end(), obj.material()) == _materials.end())
        _materials.emplace_back(obj.material());
    _objects.emplace_back(std::move(obj));
}

void Scene::add_light(PointLight obj) {
    _point_lights.emplace_back(std::move(obj));
}

Span<const SceneObject> Scene::objects() const {
    return _objects;
}

Span<const PointLight> Scene::point_lights() const {
    return _point_lights;
}

Camera& Scene::camera() {
    return _camera;
}

const Camera& Scene::camera() const {
    return _camera;
}

void Scene::set_sun(glm::vec3 direction, glm::vec3 color) {
    _sun_direction = direction;
    _sun_color = color;
}

#define MAX_INSTANCE 100

void Scene::render() const {
    // Fill and bind frame data buffer
    _data_buffer = TypedBuffer<shader::FrameData>(nullptr, 1);
    {
        auto mapping = _data_buffer.map(AccessType::WriteOnly);
        mapping[0].camera.view_proj = _camera.view_proj_matrix();
        mapping[0].point_light_count = u32(_point_lights.size());
        mapping[0].sun_color = _sun_color;
        mapping[0].sun_dir = glm::normalize(_sun_direction);
    }
    _data_buffer.bind(BufferUsage::Uniform, 0);

    // Fill and bind lights buffer
    _light_buffer = TypedBuffer<shader::PointLight>(nullptr, std::max(_point_lights.size(), size_t(1)));
    {
        auto mapping = _light_buffer.map(AccessType::WriteOnly);
        for(size_t i = 0; i != _point_lights.size(); ++i) {
            const auto& light = _point_lights[i];
            mapping[i] = {
                light.position(),
                light.radius(),
                light.color(),
                0.0f
            };
        }
    }
    _light_buffer.bind(BufferUsage::Storage, 1);

    // Render every object
    for(const SceneObject& obj : _objects) {
        // is my object seen ? (inside the camera frustum)
        //if (obj.check_frustum(camera())) // TODO: Reintroduce this once bug is crushed
            obj.render();
    }

    // failing to instance TwT
    /*for(const auto& mat : _materials) {
        glm::mat4 transforms[MAX_INSTANCE];
        int i = 0;
        SceneObject object_to_instance;
        for (const SceneObject& obj : _objects) {
            if (!obj.check_frustum(camera()) || obj.material() != mat) continue;
            transforms[i++] = obj.transform();
            object_to_instance = obj;
        }
        if (i <= 0) continue;
        object_to_instance.setup();

        TypedBuffer<glm::mat4> ssbo(transforms);
        ssbo.bind(BufferUsage::Storage, 6);


        glDrawElementsInstanced(GL_TRIANGLES, object_to_instance.index_buffer_count(), GL_UNSIGNED_INT, nullptr, i);
    }*/

}

}
