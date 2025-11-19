#include "SceneObject.h"

namespace OM3D
{

    SceneObject::SceneObject(std::shared_ptr<StaticMesh> mesh,
                             std::shared_ptr<Material> material)
        : _mesh(std::move(mesh))
        , _material(std::move(material))
    {}

    void SceneObject::render(const Camera &camera, const Frustum &frustum,
                             const bool after_z_prepass,
                             const bool backface_culling) const
    {
        if (!_material || !_mesh)
        {
            return;
        }

        _material->set_uniform(HASH("model"), transform());
        DepthTestMode original_depth_test_mode =
            _material->get_depth_test_mode();
        if (after_z_prepass)
        {
            _material->set_depth_test_mode(DepthTestMode::Equal);
        }
        _material->bind(backface_culling);
        _material->set_depth_test_mode(original_depth_test_mode);

        _mesh->draw(camera, frustum, scale(), translation());
    }

    void SceneObject::set_transform(const glm::mat4 &tr)
    {
        _transform = tr;
    }

    const glm::mat4 &SceneObject::transform() const
    {
        return _transform;
    }

    void SceneObject::set_scale(const glm::vec3 &scale)
    {
        _scale = scale;
    }

    const glm::vec3 &SceneObject::scale() const
    {
        return _scale;
    }

    void SceneObject::set_translation(const glm::vec3 &translation)
    {
        _translation = translation;
    }

    const glm::vec3 &SceneObject::translation() const
    {
        return _translation;
    }

    void SceneObject::set_rotation(const glm::mat3 &rotation)
    {
        _rotation = rotation;
    }

    const glm::mat3 &SceneObject::rotation() const
    {
        return _rotation;
    }

} // namespace OM3D
