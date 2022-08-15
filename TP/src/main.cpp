#include <glad/glad.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>

#include <graphics.h>
#include <SceneView.h>


#include <glm/vec2.hpp>


void glfw_check(bool cond) {
    if(!cond) {
        const char* err = nullptr;
        glfwGetError(&err);
        std::cerr << "GLFW error: " << err << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

void process_inputs(GLFWwindow* window, Camera& camera) {
    static glm::dvec2 mouse_pos;
    static double time;

    glm::dvec2 new_mouse_pos;
    glfwGetCursorPos(window, &new_mouse_pos.x, &new_mouse_pos.y);

    const double new_time = program_time();

    {
        glm::vec3 movement = {};
        if(glfwGetKey(window, 'W') == GLFW_PRESS) {
            movement += camera.forward();
        }
        if(glfwGetKey(window, 'S') == GLFW_PRESS) {
            movement -= camera.forward();
        }
        if(glfwGetKey(window, 'D') == GLFW_PRESS) {
            movement += camera.right();
        }
        if(glfwGetKey(window, 'A') == GLFW_PRESS) {
            movement -= camera.right();
        }
        const float speed = 10.0f;
        const float dt = float(new_time - time);
        if(movement.length() > 0.0f) {
            const glm::vec3 new_pos = camera.position() + movement * dt * speed;
            camera.set_view(glm::lookAt(new_pos, new_pos + camera.forward(), camera.up()));
        }
    }

    if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        const glm::vec2 delta = glm::vec2(mouse_pos - new_mouse_pos) * 0.01f;
        if(delta.length() > 0.0f) {
            glm::mat4 rot = glm::rotate(glm::mat4(1.0f), delta.x, glm::vec3(0.0f, 0.0f, 1.0f));
            rot = glm::rotate(rot, delta.y, camera.right());
            camera.set_view(glm::lookAt(camera.position(), camera.position() + (glm::mat3(rot) * camera.forward()), (glm::mat3(rot) * camera.up())));
        }

    }

    mouse_pos = new_mouse_pos;
    time = new_time;
}

int main(int, char**) {
    DEBUG_ASSERT([] { std::cout << "Debug asserts enabled" << std::endl; return true; }());

    glfw_check(glfwInit());
    DEFER(glfwTerminate());

    GLFWwindow* window = glfwCreateWindow(1600, 900, "TP window", nullptr, nullptr);
    glfw_check(window);
    DEFER(glfwDestroyWindow(window));

    glfwMakeContextCurrent(window);
    init_graphics();


    Scene scene;
    SceneView scene_view(&scene);

    {
        const auto r = MeshData::from_obj(std::string(data_path) + "cube.obj");
        ALWAYS_ASSERT(r.is_ok, "Unable to load mesh");
        std::shared_ptr<StaticMesh> mesh = std::make_shared<StaticMesh>(r.value);
        std::shared_ptr<Program> program = std::make_shared<Program>(Program::from_files("basic.frag", "basic.vert"));
        scene.add_object(SceneObject(std::move(mesh), std::move(program)));
    }


    for(;;) {
        glfwPollEvents();
        if(glfwWindowShouldClose(window) || glfwGetKey(window, GLFW_KEY_ESCAPE)) {
            break;
        }

        process_inputs(window, scene_view.camera());

        {
            glEnable(GL_DEPTH_TEST);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            scene_view.render();
        }

        glfwSwapBuffers(window);
    }
}
