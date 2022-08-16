#ifndef SCENE_H
#define SCENE_H

#include <SceneObject.h>
#include <PointLight.h>
#include <Camera.h>

#include <vector>

class Scene : NonMovable {

    public:
        Scene();

        void render(const Camera& camera) const;

        void add_object(SceneObject obj);
        void add_object(PointLight obj);

    private:
        std::vector<SceneObject> _objects;
        std::vector<PointLight> _point_lights;
};

#endif // SCENE_H
