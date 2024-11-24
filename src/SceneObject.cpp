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

    void SceneObject::render(const Camera& camera, const Frustum& frustum) const
    {
        if (!_material || !_mesh)
        {
            return;
        }

        const glm::mat4 t = transform();
        _material->set_uniform(HASH("model"), t);
        _material->bind();
        _mesh->draw(camera.position(), t, frustum);
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
