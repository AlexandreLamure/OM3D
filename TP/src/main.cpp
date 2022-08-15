#include <glad/glad.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>

#include <graphics.h>
#include <Program.h>
#include <StaticMesh.h>

#include <glm/gtc/matrix_transform.hpp>

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

    StaticMesh mesh;
    if(const auto r = MeshData::from_obj(std::string(data_path) + "cube.obj"); r.is_ok) {
        std::cerr << "Unable to load mesh" << std::endl;
        mesh = StaticMesh(r.value);
    } else {
        std::vector<Vertex> vertices;
        vertices.push_back({{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}});
        vertices.push_back({{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}});
        vertices.push_back({{0.0f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}});
        mesh = StaticMesh(MeshData{vertices, {0, 1, 2}});
    }
    Program program = Program::from_files("basic.frag", "basic.vert");


    const glm::vec3 cam_pos(2.0f, 2.0f, 2.0f);
    const glm::mat4 proj_matrix = glm::infinitePerspective(to_rad(60.0f), 640.0f / 480.0f, 0.001f);
    const glm::mat4 view_matrix = glm::lookAt(cam_pos, glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    const glm::mat4 view_proj = proj_matrix * view_matrix;

    for(;;) {
        glfwPollEvents();
        if(glfwWindowShouldClose(window) || glfwGetKey(window, GLFW_KEY_ESCAPE)) {
            break;
        }

        {
            glEnable(GL_DEPTH_TEST);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            program.set_uniform(HASH("view_proj"), view_proj);
            program.set_uniform(HASH("model"), glm::rotate(glm::mat4(1.0f), float(program_time()), glm::vec3(0.0f, 0.0f, 1.0f)));
            program.set_uniform(HASH("red"), float(std::sin(program_time()) * 0.5f + 0.5f));
            program.set_uniform(HASH("green"), 0.5f);
            program.set_uniform(HASH("blue"), 1.0f);
            program.bind();
            mesh.draw();
        }
        glfwSwapBuffers(window);
    }
}
