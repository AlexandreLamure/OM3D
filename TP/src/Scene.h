#ifndef SCENE_H
#define SCENE_H

#include <SceneObject.h>
#include <PointLight.h>
#include <Camera.h>

#include <vector>

namespace OM3D {

class Scene : NonMovable {

    public:
        Scene();

        static Result<Scene> from_gltf(const std::string& file_name);

        void render(const Camera& camera) const;

        void add_object(SceneObject obj);
        void add_object(PointLight obj);

    private:
        std::vector<SceneObject> _objects;
        std::vector<PointLight> _point_lights;
};

}

#endif // SCENE_H
