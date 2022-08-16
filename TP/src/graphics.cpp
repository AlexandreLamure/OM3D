#include "graphics.h"

#include <glad/glad.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

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

u32 buffer_usage_to_gl(BufferUsage usage) {
    switch(usage) {
        case BufferUsage::Attribute:
            return GL_ARRAY_BUFFER;

        case BufferUsage::Index:
            return GL_ELEMENT_ARRAY_BUFFER;

        case BufferUsage::Uniform:
            return GL_UNIFORM_BUFFER;
            
        case BufferUsage::Storage:
            return GL_SHADER_STORAGE_BUFFER;
    }

    FATAL("Unknown usage value");
}

void init_graphics() {
    ALWAYS_ASSERT(gladLoadGLLoader((GLADloadproc)(glfwGetProcAddress)), "glad initialization failed");

    std::cout << "OpenGL " << glGetString(GL_VERSION) << " initialized on " << glGetString(GL_VENDOR) << " " << glGetString(GL_RENDERER) << " using GLSL " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    glClearColor(0.5f, 0.7f, 0.8f, 0.0f);

    glDebugMessageCallback(&debug_out, nullptr);
    glEnable(GL_DEBUG_OUTPUT);
}

