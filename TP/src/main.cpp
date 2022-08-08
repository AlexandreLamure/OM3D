#include <glad/glad.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>

#include <graphics.h>
#include <Program.h>
#include <StaticMesh.h>

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

    std::vector<Vertex> vertices;
    vertices.push_back({{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}});
    vertices.push_back({{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}});
    vertices.push_back({{0.0f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}});

    const StaticMesh mesh(MeshData{vertices, {0, 1, 2}});
    Program program = Program::from_files("fixed_color.frag", "color.vert");

    for(;;) {
        glfwPollEvents();
        if(glfwWindowShouldClose(window) || glfwGetKey(window, GLFW_KEY_ESCAPE)) {
            break;
        }

        {
            glClear(GL_COLOR_BUFFER_BIT);

            program.set_uniform(HASH("red"), float(std::sin(program_time()) * 0.5f + 0.5f));
            program.set_uniform(HASH("green"), 0.5f);
            program.set_uniform(HASH("blue"), 1.0f);
            program.bind();
            mesh.draw();
        }
        glfwSwapBuffers(window);
    }
}
