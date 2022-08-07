#include <glad/glad.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "RenderContext.h"

#include <iostream>

void debug_out(GLenum, GLenum type, GLuint, GLuint sev, GLsizei, const char* msg, const void*) {
    if(sev == GL_DEBUG_SEVERITY_NOTIFICATION) {
        return;
    }

    const bool is_error = type == GL_DEBUG_TYPE_ERROR;
    (is_error ? std::cerr : std::cout) << (sev == GL_DEBUG_SEVERITY_HIGH ? "[GL][HIGH] " : "[GL] ") << msg << std::endl;

    if(is_error) {
        break_in_debugger();
    }
}



static std::string read_shader(const std::string& file_name) {
    const auto content = read_text_file(std::string(RenderContext::shader_path) + file_name);
    ALWAYS_ASSERT(content.is_ok, "Unable to read shader");
    return content.value;
}

RenderContext::RenderContext() {
    ALWAYS_ASSERT(gladLoadGLLoader((GLADloadproc)(glfwGetProcAddress)), "glad initialization failed");
    glClearColor(0.5f, 0.7f, 0.8f, 0.0f);

    glDebugMessageCallback(&debug_out, nullptr);
    glEnable(GL_DEBUG_OUTPUT);

    _screen = Program(read_shader("red.frag"), read_shader("screen.vert"));
}

RenderContext::~RenderContext() {
}

void RenderContext::draw_screen() {
    glUseProgram(_screen._handle.get());
    glDrawArrays(GL_TRIANGLES, 0, 3);
}
