#ifndef SCENE_H
#define SCENE_H

#include <Camera.h>
#include <PointLight.h>
#include <SceneObject.h>
#include <memory>
#include <vector>

#include "PassTypeEnum.h"

namespace OM3D
{

    class Scene : NonMovable
    {
    public:
        Scene();

        static Result<std::unique_ptr<Scene>>
        from_gltf(const std::string &file_name);

        void render(const enum PassType pass_type) const;

        void add_object(SceneObject obj);
        void add_light(PointLight obj);

        Span<const SceneObject> objects() const;
        Span<const PointLight> point_lights() const;

        Camera &camera();
        const Camera &camera() const;

        void set_envmap(std::shared_ptr<Texture> env);
        void set_sun(float altitude, float azimuth,
                     glm::vec3 color = glm::vec3(1.0f));
        void set_backface_culling(bool backface_culling);
        void set_frustum_culling(bool frustum_culling,
                                 float frustum_bounding_sphere_radius_coeff);

    private:
        std::vector<SceneObject> _objects;
        std::vector<PointLight> _point_lights;

        glm::vec3 _sun_direction = glm::vec3(0.2f, 1.0f, 0.1f);
        glm::vec3 _sun_color = glm::vec3(1.0f);
        bool _backface_culling = true;
        bool _frustum_culling = true;
        float _frustum_bounding_sphere_radius_coeff = 1.0f;

        std::shared_ptr<Texture> _envmap;
        Material _sky_material;

        Camera _camera;
    };

} // namespace OM3D

#endif // SCENE_H
