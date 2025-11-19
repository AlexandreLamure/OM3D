#include "SceneObject.h"

#include <glm/gtc/matrix_transform.hpp>

namespace OM3D
{

    SceneObject::SceneObject(std::shared_ptr<StaticMesh> mesh,
                             std::shared_ptr<Material> material)
        : _mesh(std::move(mesh))
        , _material(std::move(material))
    {}

    void SceneObject::render(const Camera &camera, const Frustum &frustum,
                             const bool after_z_prepass) const
    {
        if (!_material || !_mesh)
        {
            return;
        }

        _material->set_uniform(HASH("model"), transform());
        DepthTestMode original_depth_test_mode = _material->get_depth_test_mode();
        if (after_z_prepass)
        {
            _material->set_depth_test_mode(DepthTestMode::Equal);
        }
        _material->bind();
        _material->set_depth_test_mode(original_depth_test_mode);

        _mesh->draw(camera, frustum);
    }

    void SceneObject::set_transform(const glm::mat4 &tr)
    {
        _transform = tr;
    }

    const glm::mat4 &SceneObject::transform() const
    {
        return _transform;
    }

} // namespace OM3D
