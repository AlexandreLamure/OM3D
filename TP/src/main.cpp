#include <glad/glad.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>

#include <graphics.h>
#include <SceneView.h>

void glfw_check(bool cond) {
    if(!cond) {
        const char* err = nullptr;
        glfwGetError(&err);
        std::cerr << "GLFW error: " << err << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

int main(int, char**) {
    DEBUG_ASSERT([] { std::cout << "Debug asserts enabled" << std::endl; return true; }());

    glfw_check(glfwInit());
    DEFER(glfwTerminate());

    GLFWwindow* window = glfwCreateWindow(640, 480, "TP window", nullptr, nullptr);
    glfw_check(window);
    DEFER(glfwDestroyWindow(window));

    glfwMakeContextCurrent(window);
    init_graphics();


    Scene scene;
    SceneView scene_view(&scene);

    {
        std::shared_ptr<StaticMesh> mesh;
        if(const auto r = MeshData::from_obj(std::string(data_path) + "cube.obj"); r.is_ok) {
            std::cerr << "Unable to load mesh" << std::endl;
            mesh = std::make_shared<StaticMesh>(r.value);
        } else {
            std::vector<Vertex> vertices;
            vertices.push_back({{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}});
            vertices.push_back({{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}});
            vertices.push_back({{0.0f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}});
            mesh = std::make_shared<StaticMesh>(MeshData{vertices, {0, 1, 2}});
        }
        std::shared_ptr<Program> program = std::make_shared<Program>(Program::from_files("basic.frag", "basic.vert"));
        scene.add_object(SceneObject(std::move(mesh), std::move(program)));
    }


    for(;;) {
        glfwPollEvents();
        if(glfwWindowShouldClose(window) || glfwGetKey(window, GLFW_KEY_ESCAPE)) {
            break;
        }

        {
            glEnable(GL_DEPTH_TEST);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            scene_view.render();
        }
        glfwSwapBuffers(window);
    }
}
