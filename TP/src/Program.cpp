#include "Program.h"

#include <glad/glad.h>

static std::string read_shader(const std::string& file_name) {
    const auto content = read_text_file(std::string(shader_path) + file_name);
    ALWAYS_ASSERT(content.is_ok, "Unable to read shader");
    return content.value;
}

static GLuint create_shader(const std::string& src, GLenum type) {
    const GLuint handle = glCreateShader(type);

    const int len = int(src.size());
    const char* c_str = src.c_str();

    glShaderSource(handle, 1, &c_str, &len);
    glCompileShader(handle);

    int res = 0;
    glGetShaderiv(handle, GL_COMPILE_STATUS, &res);
    if(!res) {
        int len = 0;
        char log[1024] = {};
        glGetShaderInfoLog(handle, sizeof(log), &len, log);
        FATAL(log);
    }

    return handle;
}

static void link_program(GLuint handle) {
    glLinkProgram(handle);

    int res = 0;
    glGetProgramiv(handle, GL_LINK_STATUS, &res);
    if(!res) {
        int len = 0;
        char log[1024] = {};
        glGetProgramInfoLog(handle, sizeof(log), &len, log);
        FATAL(log);
    }
}

Program::Program(const std::string& frag, const std::string& vert) : _handle(glCreateProgram()) {
    const GLuint vert_handle = create_shader(vert, GL_VERTEX_SHADER);
    const GLuint frag_handle = create_shader(frag, GL_FRAGMENT_SHADER);

    glAttachShader(_handle.get(), vert_handle);
    glAttachShader(_handle.get(), frag_handle);

    link_program(_handle.get());

    glDeleteShader(vert_handle);
    glDeleteShader(frag_handle);
}

Program::~Program() {
    if(_handle.is_valid()) {
        glDeleteProgram(_handle.get());
    }
}

void Program::bind() const {
    glUseProgram(_handle.get());
}

Program Program::from_files(const std::string& frag, const std::string& vert) {
    return Program(read_shader(frag), read_shader(vert));
}
