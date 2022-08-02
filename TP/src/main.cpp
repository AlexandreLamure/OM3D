#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>

#include <utils.h>

void glfw_check(bool cond) {
    if(!cond) {
        const char* err = nullptr;
        glfwGetError(&err);
        std::cerr << "GLFW error: " << err << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

void render_gl(GLFWwindow* window) {
    glfwMakeContextCurrent(window);
    glClear(GL_COLOR_BUFFER_BIT);
}

int main(int, char**) {
    glfw_check(glfwInit());
    DEFER(glfwTerminate());

    GLFWwindow* window = glfwCreateWindow(640, 480, "TP window", nullptr, nullptr);
    glfw_check(window);
    DEFER(glfwDestroyWindow(window));

    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);

    glClearColor(0.5f, 0.7f, 0.8f, 0.0f);

    for(;;) {
        glfwPollEvents();
        if(glfwWindowShouldClose(window) || glfwGetKey(window, GLFW_KEY_ESCAPE)) {
            break;
        }

        render_gl(window);
        glfwSwapBuffers(window);
    }
}
