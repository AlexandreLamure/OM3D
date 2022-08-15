#ifndef SCENEOBJECT_H
#define SCENEOBJECT_H

#include <StaticMesh.h>
#include <Program.h>

#include <memory>

#include <glm/matrix.hpp>

class SceneObject : NonCopyable {

    public:
        SceneObject(std::shared_ptr<StaticMesh> mesh = nullptr, std::shared_ptr<Program> shader = nullptr);

        void render() const;

        const glm::mat4& transform() const;

    private:
        glm::mat4 _transform = glm::mat4(1.0f);

        std::shared_ptr<StaticMesh> _mesh;
        std::shared_ptr<Program> _shader;
};

#endif // SCENEOBJECT_H
