#include "Scene.h"

#include <TypedBuffer.h>
#include <iostream>
#include <shader_structs.h>

namespace OM3D
{

    Scene::Scene()
    {
        _sky_material.set_program(
            Program::from_files("sky.frag", "screen.vert"));
        _sky_material.set_depth_test_mode(DepthTestMode::None);

        _envmap = std::make_shared<Texture>(
            Texture::empty_cubemap(4, ImageFormat::RGBA8_UNORM));
    }

    void Scene::add_object(SceneObject obj)
    {
        _objects.emplace_back(std::move(obj));
    }

    void Scene::add_light(PointLight obj)
    {
        _point_lights.emplace_back(std::move(obj));
    }

    Span<const SceneObject> Scene::objects() const
    {
        return _objects;
    }

    Span<const PointLight> Scene::point_lights() const
    {
        return _point_lights;
    }

    Camera &Scene::camera()
    {
        return _camera;
    }

    const Camera &Scene::camera() const
    {
        return _camera;
    }

    void Scene::set_envmap(std::shared_ptr<Texture> env)
    {
        _envmap = std::move(env);
    }

    void Scene::set_sun(float altitude, float azimuth, glm::vec3 color)
    {
        // Convert from degrees to radians
        const float alt = glm::radians(altitude);
        const float azi = glm::radians(azimuth);
        // Convert from polar to cartesian
        _sun_direction =
            glm::vec3(sin(azi) * cos(alt), sin(alt), cos(azi) * cos(alt));
        _sun_color = color;
    }

    void Scene::set_backface_culling(const bool backface_culling)
    {
        _backface_culling = backface_culling;
    }

    void
    Scene::set_frustum_culling(const bool frustum_culling,
                               const float frustum_bounding_sphere_radius_coeff)
    {
        _frustum_culling = frustum_culling;
        _frustum_bounding_sphere_radius_coeff =
            frustum_bounding_sphere_radius_coeff;
    }

    void Scene::render(const enum PassType pass_type)
    {
        const glm::mat4 view_matrix = _camera.view_matrix();
        const glm::mat4 projection_matrix = _camera.projection_matrix();
        if (pass_type == PassType::SHADOW
            || pass_type == PassType::SHADOW_NO_DEPTH)
        {
            // const auto [average_position, scene_radius] =
            //     get_scene_center_and_radius();
            glm::vec3 average_position = glm::vec3(0, 0, 0);

            float real_scene_radius = 10.f;
            glm::vec3 light_dir = _sun_direction;
            glm::vec3 light_position = average_position - light_dir;

            glm::vec3 global_up = glm::vec3(0.0, 1.0, 0.0);
            glm::vec3 up_camera =
                glm::cross(glm::cross(glm::normalize(light_dir), global_up),
                           glm::normalize(light_dir));

            _camera.set_view(glm::lookAt(
                light_position, light_position - light_dir, up_camera));
            _camera.set_proj(Camera::orthographic(
                -5 * real_scene_radius, 5 * real_scene_radius,
                -5 * real_scene_radius, 5 * real_scene_radius,
                real_scene_radius * -10.0f, real_scene_radius * 10.0f));

            std::cout << "Camera position: " << _camera.position()[0] << ", "
                      << _camera.position()[1] << ", " << _camera.position()[2]
                      << "\n";
            std::cout << "light position: " << light_position[0] << ", "
                      << light_position[1] << ", " << light_position[2] << "\n";
            std::cout << "Scene center: " << average_position[0] << ", "
                      << average_position[1] << ", " << average_position[2]
                      << "\n";
        }

        // Fill and bind frame data buffer
        TypedBuffer<shader::FrameData> buffer(nullptr, 1);
        {
            auto mapping = buffer.map(AccessType::WriteOnly);
            mapping[0].camera.view_proj = _camera.view_proj_matrix();
            mapping[0].camera.inv_view_proj =
                glm::inverse(_camera.view_proj_matrix());
            mapping[0].camera.position = _camera.position();
            mapping[0].point_light_count = u32(_point_lights.size());
            mapping[0].sun_color = _sun_color;
            mapping[0].sun_dir = glm::normalize(_sun_direction);
        }
        buffer.bind(BufferUsage::Uniform, 0);

        // Fill and bind lights buffer
        TypedBuffer<shader::PointLight> light_buffer(
            nullptr, std::max(_point_lights.size(), size_t(1)));
        {
            auto mapping = light_buffer.map(AccessType::WriteOnly);
            for (size_t i = 0; i != _point_lights.size(); ++i)
            {
                const auto &light = _point_lights[i];
                mapping[i] = { light.position(), light.radius(), light.color(),
                               0.0f };
            }
        }
        light_buffer.bind(BufferUsage::Storage, 1);

        // Bind envmap
        DEBUG_ASSERT(_envmap && !_envmap->is_null());
        _envmap->bind(4);

        // Bind brdf lut needed for lighting to scene rendering shaders
        brdf_lut().bind(5);

        // Render the sky
        _sky_material.bind(false);
        draw_full_screen_triangle();

        Frustum frustum = camera().build_frustum();
        frustum._culling_enabled = _frustum_culling;
        frustum._culling_bounding_sphere_coeff =
            _frustum_bounding_sphere_radius_coeff;

        bool after_z_prepass = pass_type == PassType::MAIN;

        // Render every object

        {
            // Opaque first
            for (const SceneObject &obj : _objects)
            {
                if (obj.material().is_opaque())
                {
                    obj.render(camera(), frustum, after_z_prepass,
                               _backface_culling);
                }
            }

            // Transparent after
            for (const SceneObject &obj : _objects)
            {
                if (!obj.material().is_opaque())
                {
                    obj.render(camera(), frustum, after_z_prepass,
                               _backface_culling);
                }
            }

            for (const SceneObject &obj : _objects)
            {
                obj.render(camera(), frustum, after_z_prepass,
                           _backface_culling);
            }
        }
        if (pass_type == PassType::SHADOW
            || pass_type == PassType::SHADOW_NO_DEPTH)
        {
            // restore camera view and matrix
            _camera.set_view(view_matrix);
            _camera.set_proj(projection_matrix);
        }
    }

    std::pair<glm::vec3, float> Scene::get_scene_center_and_radius()
    {
        glm::vec3 sum_of_positions = { 0.0, 0.0, 0.0 };
        for (const SceneObject &object : _objects)
        {
            // get the translation part of the transform of an object
            sum_of_positions +=
                object.get_static_mesh().get_bounding_sphere().get_center();
        }
        const float div = 1.0f / _objects.size();
        glm::vec3 average_position =
            glm::vec3(sum_of_positions[0] * div, sum_of_positions[1] * div,
                      sum_of_positions[2] * div);
        float squared_scene_radius = 0;
        for (const SceneObject &object : _objects)
        {
            const glm::vec3 to_center = average_position - object.translation();
            const float squared_object_size =
                glm::dot(object.scale(), object.scale());
            squared_scene_radius =
                std::max(squared_scene_radius,
                         glm::dot(to_center, to_center) + squared_object_size);
        }
        return { average_position, std::sqrt(squared_scene_radius) };
    }

} // namespace OM3D
