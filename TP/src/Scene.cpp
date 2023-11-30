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
    TypedBuffer<shader::FrameData> buffer(nullptr, 1);
    {
        auto mapping = buffer.map(AccessType::WriteOnly);
        mapping[0].camera.view_proj = _camera.view_proj_matrix();
        mapping[0].point_light_count = u32(_point_lights.size());
        mapping[0].sun_color = _sun_color;
        mapping[0].sun_dir = glm::normalize(_sun_direction);
    }
    buffer.bind(BufferUsage::Uniform, 0);

    // Fill and bind lights buffer
    TypedBuffer<shader::PointLight> light_buffer(nullptr, std::max(_point_lights.size(), size_t(1)));
    {
        auto mapping = light_buffer.map(AccessType::WriteOnly);
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
    light_buffer.bind(BufferUsage::Storage, 1);

    // for instancing
    // get the number of unique meshes
    std::vector<std::shared_ptr<Material>> materials;

    // Render every object
//    for(const SceneObject& obj : _objects) {
//        // is my object seen ? (inside the camera frustum)
//        if (obj.check_frustum(camera())) {
//            //obj.render();
//            if (std::find(materials.begin(), materials.end(), obj.material()) == materials.end())
//                materials.emplace_back(obj.material());
//        }
//
//    }

    for(const auto& mat : materials) {
        mat->bind();
        glm::mat4 transforms[MAX_INSTANCE];
        int i = 0;
        for (const SceneObject& obj : _objects) {
            if (!obj.check_frustum(camera()) || obj.material() != mat) continue;
            transforms[i++] = obj.transform();
        }
        GLuint transformBuffer;
        if (i <= 0) continue;
        glGenBuffers(1, &transformBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, transformBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4), transforms, GL_STATIC_DRAW);

        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));
        glEnableVertexAttribArray(6);

        glVertexAttribDivisor(3, 1);
        glVertexAttribDivisor(4, 1);
        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);

        glDrawArraysInstanced(GL_TRIANGLES, 0, 36, i);
    }

}

}
