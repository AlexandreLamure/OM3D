#include "SceneObject.h"

#include <glm/gtc/matrix_transform.hpp>

#include "Camera.h"

namespace OM3D
{

    SceneObject::SceneObject(std::shared_ptr<StaticMesh> mesh,
                             std::shared_ptr<Material> material)
        : _mesh(std::move(mesh))
        , _material(std::move(material))
    {}

    void SceneObject::render(const Camera& camera, bool z_prepass) const
    {
        if (!_material || !_mesh)
        {
            return;
        }

        if (!z_prepass)
        {
            _material->set_uniform(HASH("model"), transform());
            _material->bind();
        }
        _mesh->draw(camera);
    }

    void SceneObject::set_transform(const glm::mat4& tr)
    {
        _transform = tr;
    }

    const glm::mat4& SceneObject::transform() const
    {
        return _transform;
    }

} // namespace OM3D
