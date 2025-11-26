#ifndef SCENEOBJECT_H
#define SCENEOBJECT_H

#include <Material.h>
#include <StaticMesh.h>
#include <glm/matrix.hpp>
#include <memory>

namespace OM3D
{

    class SceneObject
    {
    public:
        SceneObject(std::shared_ptr<StaticMesh> mesh = nullptr,
                    std::shared_ptr<Material> material = nullptr);

        void render(const Camera &camera, const Frustum &frustum,
                    bool after_z_prepass, bool backface_culling) const;

        const Material &material() const;

        void set_transform(const glm::mat4 &tr);
        void set_scale(const glm::vec3 &scale);
        void set_translation(const glm::vec3 &translation);
        void set_rotation(const glm::mat3 &rotation);

        const glm::mat4 &transform() const;
        const glm::vec3 &scale() const;
        const glm::vec3 &translation() const;
        const glm::mat3 &rotation() const;

        const StaticMesh &get_static_mesh() const;

    private:
        glm::mat4 _transform = glm::mat4(1.0f);
        glm::vec3 _scale = glm::vec3(1.0f);
        glm::vec3 _translation = glm::vec3(0.0f);
        glm::mat3 _rotation = glm::identity<glm::mat3>();

        std::shared_ptr<StaticMesh> _mesh;
        std::shared_ptr<Material> _material;
    };

} // namespace OM3D

#endif // SCENEOBJECT_H
