#include "graphics.h"

#include <glad/glad.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>

namespace OM3D {

void debug_out(GLenum, GLenum type, GLuint, GLenum sev, GLsizei, const char* msg, const void*) {
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

u32 access_type_to_gl(AccessType access) {
    switch(access) {
        case AccessType::WriteOnly:
            return GL_WRITE_ONLY;

        case AccessType::ReadOnly:
            return GL_READ_ONLY;

        case AccessType::ReadWrite:
            return GL_READ_WRITE;
    }

    FATAL("Unknown access type value");
}


u32 align_up_to(u32 val, u32 up_to) {
    if(const u32 diff = val % up_to) {
        return val + up_to - diff;
    }
    return val;
}

static GLuint global_vao = 0;

void init_graphics() {
    ALWAYS_ASSERT(gladLoadGLLoader((GLADloadproc)(glfwGetProcAddress)), "glad initialization failed");

    std::cout << "OpenGL " << glGetString(GL_VERSION) << " initialized on " << glGetString(GL_VENDOR) << " " << glGetString(GL_RENDERER) << " using GLSL " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    glClearColor(0.5f, 0.7f, 0.8f, 0.0f);

    {
        glDebugMessageCallback(&debug_out, nullptr);

        glEnable(GL_DEBUG_OUTPUT);
#ifdef OS_WIN
        if(running_in_debugger()) {
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        }
#endif
    }

    {
        // Set depth range to [0; 1] instead of the default [-1; 1]
        glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
        // We use reverse-Z so far is 0
        glClearDepthf(0.0f);
    }

    glGenVertexArrays(1, &global_vao);
    glBindVertexArray(global_vao);

}

}
