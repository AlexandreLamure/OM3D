#include "Program.h"

#include <glad/gl.h>

#include <algorithm>
#include <unordered_set>
#include <unordered_map>

namespace OM3D {

static std::string read_shader(const std::string& file_name, Span<const std::string> defines = {}) {
    const std::string full_path = std::string(shader_path) + file_name;

    auto content = read_text_file(full_path);
    if (!content.is_ok) {
        FATAL((std::string("Unable to read shader: \"") + full_path + '"').c_str());
    }

    bool define_added = false;
    auto add_defines = [&]() {
        if(define_added || defines.is_empty()) {
            return std::string();
        }
        define_added = true;
        std::string defs = "\n";
        for(const std::string& def : defines) {
            defs += "#define " + def + " 1\n";
        }
        return defs;
    };

    std::string shader(std::move(content.value));
    std::unordered_set<std::string> includes;
    for(size_t i = 0; i < shader.size();) {
        const auto endl = shader.find('\n', i);
        if(endl == std::string::npos) {
            break;
        }

        const std::string_view full_line = std::string_view(shader).substr(i, endl - i);
        std::string_view line = full_line;
        auto trim = [&] {
            while(!line.empty() && std::isspace(line.front())) {
                line = line.substr(1);
            }
        };

        trim();
        if(!line.empty() && line.front() == '#') {
            line = line.substr(1);
            trim();
            if(line.substr(0, 7) == "version") {
                shader = shader.substr(0, endl) + add_defines() + shader.substr(endl);
            } else if(line.substr(0, 7) == "include") {
                line = line.substr(7);
                trim();
                if(!line.empty()) {
                    const char delim = line.front();
                    // TODO: parse <>
                    const auto end = line.find(delim, 1);
                    if(end != line.size() - 1 || delim != '"') {
                        FATAL((std::string("Unable to parse shader include: \"") + std::string(full_line) + '"').c_str());
                    }

                    const std::string include_file(line.substr(1, end - 1));
                    std::string include_content = add_defines();
                    if(includes.find(include_file) == includes.end()) {
                        includes.insert(include_file);
                        auto content = read_text_file(std::string(shader_path) + include_file);
                        if(!content.is_ok) {
                            FATAL((std::string("Shader include not found: \"") + std::string(full_line) + '"').c_str());
                        }
                        include_content = std::move(content.value);
                    }

                    shader = shader.substr(0, i) + include_content + shader.substr(endl);
                    continue;
                }
            }
        }

        i = endl + 1;
    }

    return shader;
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

    fetch_uniform_locations();
}

Program::Program(const std::string& comp) : _handle(glCreateProgram()), _is_compute(true) {
    const GLuint comp_handle = create_shader(comp, GL_COMPUTE_SHADER);

    glAttachShader(_handle.get(), comp_handle);

    link_program(_handle.get());

    glDeleteShader(comp_handle);

    fetch_uniform_locations();
}

void Program::fetch_uniform_locations() {
    int uniform_count = 0;
    glGetProgramiv(_handle.get(), GL_ACTIVE_UNIFORMS, &uniform_count);

    for(int i = 0; i != uniform_count; ++i) {
        char name[1024] = {};
        int len = 0;
        int discard = 0;
        GLenum type = GL_NONE;

        glGetActiveUniform(_handle.get(), i, sizeof(name), &len, &discard, &type, name);

        _uniform_locations.emplace_back(UniformLocationInfo{str_hash(name), glGetUniformLocation(_handle.get(), name)});
    }

    std::sort(_uniform_locations.begin(), _uniform_locations.end());
    ALWAYS_ASSERT(std::unique(_uniform_locations.begin(), _uniform_locations.end()) == _uniform_locations.end(), "Duplicated uniform hash");

}

Program::~Program() {
    if(_handle.is_valid()) {
        glDeleteProgram(_handle.get());
    }
}

void Program::bind() const {
    glUseProgram(_handle.get());
}

bool Program::is_compute() const {
    return _is_compute;
}

std::shared_ptr<Program> Program::from_file(const std::string& comp, Span<const std::string> defines) {
    static std::unordered_map<std::vector<std::string>, std::weak_ptr<Program>, CollectionHasher<std::vector<std::string>>> loaded;

    std::vector<std::string> key(defines.begin(), defines.end());
    key.emplace_back(comp);

    auto& weak_program = loaded[key];
    auto program = weak_program.lock();
    if(!program) {
        program = std::make_shared<Program>(read_shader(comp, defines));
        weak_program = program;
    }
    return program;
}

std::shared_ptr<Program> Program::from_files(const std::string& frag, const std::string& vert, Span<const std::string> defines) {
    static std::unordered_map<std::vector<std::string>, std::weak_ptr<Program>, CollectionHasher<std::vector<std::string>>> loaded;

    std::vector<std::string> key(defines.begin(), defines.end());
    key.emplace_back(frag);
    key.emplace_back(vert);

    auto& weak_program = loaded[key];
    auto program = weak_program.lock();
    if(!program) {
        program = std::make_shared<Program>(read_shader(frag, defines), read_shader(vert, defines));
        weak_program = program;
    }
    return program;
}

int Program::find_location(u32 hash) {
    const auto it = std::lower_bound(_uniform_locations.begin(), _uniform_locations.end(), UniformLocationInfo{hash, 0});
    return (it == _uniform_locations.end() || it->name_hash != hash) ? -1 : it->location;
}



void Program::set_uniform(u32 name_hash, u32 value) {
    if(const int loc = find_location(name_hash); loc >= 0) {
        glProgramUniform1ui(_handle.get(), loc, value);
    }
}

void Program::set_uniform(u32 name_hash, float value) {
    if(const int loc = find_location(name_hash); loc >= 0) {
        glProgramUniform1f(_handle.get(), loc, value);
    }
}

void Program::set_uniform(u32 name_hash, glm::vec2 value) {
    if(const int loc = find_location(name_hash); loc >= 0) {
        glProgramUniform2f(_handle.get(), loc, value.x, value.y);
    }
}

void Program::set_uniform(u32 name_hash, glm::vec3 value) {
    if(const int loc = find_location(name_hash); loc >= 0) {
        glProgramUniform3f(_handle.get(), loc, value.x, value.y, value.z);
    }
}

void Program::set_uniform(u32 name_hash, glm::vec4 value) {
    if(const int loc = find_location(name_hash); loc >= 0) {
        glProgramUniform4f(_handle.get(), loc, value.x, value.y, value.z, value.w);
    }
}

void Program::set_uniform(u32 name_hash, const glm::mat2& value) {
    if(const int loc = find_location(name_hash); loc >= 0) {
        glProgramUniformMatrix2fv(_handle.get(), loc, 1, false, reinterpret_cast<const float*>(&value));
    }
}

void Program::set_uniform(u32 name_hash, const glm::mat3& value) {
    if(const int loc = find_location(name_hash); loc >= 0) {
        glProgramUniformMatrix3fv(_handle.get(), loc, 1, false, reinterpret_cast<const float*>(&value));
    }
}

void Program::set_uniform(u32 name_hash, const glm::mat4& value) {
    if(const int loc = find_location(name_hash); loc >= 0) {
        glProgramUniformMatrix4fv(_handle.get(), loc, 1, false, reinterpret_cast<const float*>(&value));
    }
}

void Program::set_uniform(u32 name_hash, u64 value) {
    DEBUG_ASSERT(bindless_enabled());
    DEBUG_ASSERT(glProgramUniformHandleui64ARB);
    if(const int loc = find_location(name_hash); loc >= 0) {
        glProgramUniformHandleui64ARB(_handle.get(), loc, value);
    }
}

}
