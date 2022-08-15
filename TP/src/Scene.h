#ifndef SCENE_H
#define SCENE_H

#include <SceneObject.h>
#include <Camera.h>

#include <vector>

class Scene : NonMovable {

    public:
        Scene();

        void render(const Camera& camera) const;

        void add_object(SceneObject obj);

    private:
        std::vector<SceneObject> _objects;
};

#endif // SCENE_H
