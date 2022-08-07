#include <glad/glad.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>

#include <RenderContext.h>

void glfw_check(bool cond) {
    if(!cond) {
        const char* err = nullptr;
        glfwGetError(&err);
        std::cerr << "GLFW error: " << err << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

int main(int, char**) {
    glfw_check(glfwInit());
    DEFER(glfwTerminate());

    GLFWwindow* window = glfwCreateWindow(640, 480, "TP window", nullptr, nullptr);
    glfw_check(window);
    DEFER(glfwDestroyWindow(window));

    glfwMakeContextCurrent(window);
    RenderContext ctx;

    for(;;) {
        glfwPollEvents();
        if(glfwWindowShouldClose(window) || glfwGetKey(window, GLFW_KEY_ESCAPE)) {
            break;
        }

        {
            glClear(GL_COLOR_BUFFER_BIT);
            ctx.draw_screen();
        }
        glfwSwapBuffers(window);
    }
}
