#include "graphics.h"

#include "ImageFormat.h"
#include "Texture.h"
#include "Program.h"
#include "TimestampQuery.h"

#include <glad/gl.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>
#include <cstring>

namespace OM3D {

Texture brdf_lut_texture;

struct {
    std::shared_ptr<Texture> black;
    std::shared_ptr<Texture> white;
    std::shared_ptr<Texture> normal;
    std::shared_ptr<Texture> metal_rough;
} default_textures;

bool audit_bindings_before_draw = false;

void debug_out(GLenum, GLenum type, GLuint, GLenum sev, GLsizei, const char* msg, const void*) {
    if(sev == GL_DEBUG_SEVERITY_NOTIFICATION) {
        return;
    }

    const bool is_error = (type == GL_DEBUG_TYPE_ERROR || type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR);
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



bool bindless_enabled() {
    return GLAD_GL_ARB_bindless_texture != 0;
}

void init_graphics() {
    ALWAYS_ASSERT(gladLoadGL(glfwGetProcAddress), "glad initialization failed");

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

    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_FRAMEBUFFER_SRGB);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    GLuint global_vao = 0;
    glGenVertexArrays(1, &global_vao);
    glBindVertexArray(global_vao);

    {
        brdf_lut_texture = Texture(glm::uvec2(256), ImageFormat::RG16_UNORM, WrapMode::Clamp);

        std::shared_ptr<Program> brdf_program = Program::from_file("brdf.comp");
        DEBUG_ASSERT(brdf_program && brdf_program->is_compute());

        brdf_program->bind();
        brdf_lut_texture.bind_as_image(0, AccessType::WriteOnly);
        glDispatchCompute(512 / 8, 512 / 8, 1);
    }

    {
        TextureData data;
        data.format = ImageFormat::RGBA8_UNORM;
        data.size = glm::uvec2(2, 2);
        data.data = std::make_unique<u8[]>(16);

        {
            std::memset(data.data.get(), 0, 16);
            default_textures.black = std::make_shared<Texture>(data);
        }
        {
            std::memset(data.data.get(), 255, 16);
            default_textures.white = std::make_shared<Texture>(data);
        }
        {
            std::memset(data.data.get(), 0, 16);
            for(size_t i = 0; i != 4; ++i) {
                data.data[i * 4 + 0] = 127;
                data.data[i * 4 + 1] = 127;
                data.data[i * 4 + 2] = 255;
            }
            default_textures.normal = std::make_shared<Texture>(data);
        }
        {
            std::memset(data.data.get(), 0, 16);
            for(size_t i = 0; i != 4; ++i) {
                data.data[i * 4 + 1] = u8(255.0f * 0.6f);
                data.data[i * 4 + 2] = 0;
            }
            default_textures.metal_rough = std::make_shared<Texture>(data);
        }
    }
}

void destroy_graphics() {
    brdf_lut_texture = {};
    default_textures = {};
    profile::destroy_profile();
}

const Texture& brdf_lut() {
    return brdf_lut_texture;
}


void draw_full_screen_triangle() {
    if(audit_bindings_before_draw) {
        audit_bindings();
    }

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(4);

    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void blit_to_screen(const Texture& tex) {
    const std::shared_ptr<Program> blit_program = Program::from_files("passthrough.frag", "screen.vert");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST); // In case glfw gives us a depth buffer

    int viewport[4] = {};
    glGetIntegerv(GL_VIEWPORT, viewport);

    blit_program->bind();
    tex.bind(0);
    draw_full_screen_triangle();
}

std::shared_ptr<Texture> default_black_texture() {
    return default_textures.black;
}

std::shared_ptr<Texture> default_white_texture() {
    return default_textures.white;
}

std::shared_ptr<Texture> default_normal_texture() {
    return default_textures.normal;
}

std::shared_ptr<Texture> default_metal_rough_texture() {
    return default_textures.metal_rough;
}





[[maybe_unused]]
static bool is_sampler_type(GLenum type) {
    switch(type) {
        case GL_SAMPLER_1D:
        case GL_SAMPLER_2D:
        case GL_SAMPLER_3D:
        case GL_SAMPLER_CUBE:
        case GL_SAMPLER_1D_SHADOW:
        case GL_SAMPLER_2D_SHADOW:
        case GL_SAMPLER_1D_ARRAY:
        case GL_SAMPLER_2D_ARRAY:
        case GL_SAMPLER_1D_ARRAY_SHADOW:
        case GL_SAMPLER_2D_ARRAY_SHADOW:
        case GL_SAMPLER_2D_MULTISAMPLE:
        case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_SAMPLER_CUBE_SHADOW:
        case GL_SAMPLER_BUFFER:
        case GL_SAMPLER_2D_RECT:
        case GL_SAMPLER_2D_RECT_SHADOW:
        case GL_INT_SAMPLER_1D:
        case GL_INT_SAMPLER_2D:
        case GL_INT_SAMPLER_3D:
        case GL_INT_SAMPLER_CUBE:
        case GL_INT_SAMPLER_1D_ARRAY:
        case GL_INT_SAMPLER_2D_ARRAY:
        case GL_INT_SAMPLER_2D_MULTISAMPLE:
        case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_INT_SAMPLER_BUFFER:
        case GL_INT_SAMPLER_2D_RECT:
        case GL_UNSIGNED_INT_SAMPLER_1D:
        case GL_UNSIGNED_INT_SAMPLER_2D:
        case GL_UNSIGNED_INT_SAMPLER_3D:
        case GL_UNSIGNED_INT_SAMPLER_CUBE:
        case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_BUFFER:
        case GL_UNSIGNED_INT_SAMPLER_2D_RECT:
            return true;

        default:
            return false;
    }
}

[[maybe_unused]]
static bool is_image_type(GLenum type) {
    switch(type) {
        case GL_IMAGE_1D:
        case GL_IMAGE_2D:
        case GL_IMAGE_3D:
        case GL_IMAGE_2D_RECT:
        case GL_IMAGE_CUBE:
        case GL_IMAGE_BUFFER:
        case GL_IMAGE_1D_ARRAY:
        case GL_IMAGE_2D_ARRAY:
        case GL_IMAGE_2D_MULTISAMPLE:
        case GL_IMAGE_2D_MULTISAMPLE_ARRAY:
        case GL_INT_IMAGE_1D:
        case GL_INT_IMAGE_2D:
        case GL_INT_IMAGE_3D:
        case GL_INT_IMAGE_2D_RECT:
        case GL_INT_IMAGE_CUBE:
        case GL_INT_IMAGE_BUFFER:
        case GL_INT_IMAGE_1D_ARRAY:
        case GL_INT_IMAGE_2D_ARRAY:
        case GL_INT_IMAGE_2D_MULTISAMPLE:
        case GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY:
        case GL_UNSIGNED_INT_IMAGE_1D:
        case GL_UNSIGNED_INT_IMAGE_2D:
        case GL_UNSIGNED_INT_IMAGE_3D:
        case GL_UNSIGNED_INT_IMAGE_2D_RECT:
        case GL_UNSIGNED_INT_IMAGE_CUBE:
        case GL_UNSIGNED_INT_IMAGE_BUFFER:
        case GL_UNSIGNED_INT_IMAGE_1D_ARRAY:
        case GL_UNSIGNED_INT_IMAGE_2D_ARRAY:
        case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE:
        case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY:
            return true;

        default:
            return false;
    }
}


[[maybe_unused]]
static bool is_cube_type(GLenum type) {
    switch(type) {
        case GL_IMAGE_CUBE:
        case GL_SAMPLER_CUBE:
            return true;

        default:
            return false;
    }
}

void audit_bindings() {
    auto get = [](GLenum e) {
        int v = 0;
        glGetIntegerv(e , &v);
        return v;
    };

    auto get_at = [](GLenum e, unsigned index) {
        int v = 0;
        glGetIntegeri_v(e , index, &v);
        return v;
    };

    const int current_program = get(GL_CURRENT_PROGRAM);
    ALWAYS_ASSERT(current_program && glIsProgram(current_program), "Bound program is destroyed or invalid");


    auto get_resource_count = [=](GLenum type) {
        int count = 0;
        glGetProgramInterfaceiv(current_program, type, GL_ACTIVE_RESOURCES, &count);
        return count;
    };

    auto get_resource_property = [=](GLenum res_type, GLenum property, unsigned index) {
        int value = 0;
        glGetProgramResourceiv(current_program, res_type, index, 1, &property, 1, nullptr, &value);
        return value;
    };

    {
        const int uniform_count = get_resource_count(GL_UNIFORM);
        for(int i = 0; i != uniform_count; ++i) {
            const GLenum type = get_resource_property(GL_UNIFORM, GL_TYPE, i);

            if(is_sampler_type(type) || is_image_type(type)) {
                const int location = get_resource_property(GL_UNIFORM, GL_LOCATION, i);

                unsigned index = 0;
                glGetUniformuiv(current_program, location, &index);
                ALWAYS_ASSERT(glIsTexture(get_at(is_cube_type(type) ? GL_TEXTURE_BINDING_CUBE_MAP : GL_TEXTURE_BINDING_2D, index)), "Bound texture is destroyed or invalid");
            }
        }
    }

    {
        const int block_count = get_resource_count(GL_UNIFORM_BLOCK);
        for(int i = 0; i != block_count; ++i) {
            const unsigned binding = get_resource_property(GL_UNIFORM_BLOCK, GL_BUFFER_BINDING, i);

            const int buffer = get_at(GL_UNIFORM_BUFFER_BINDING, binding);
            ALWAYS_ASSERT(buffer && glIsBuffer(buffer), "Bound uniform buffer is destroyed or invalid");

            void* mapping = nullptr;
            glGetNamedBufferPointerv(buffer, GL_BUFFER_MAP_POINTER, &mapping);
            ALWAYS_ASSERT(!mapping, "Uniform buffer is still mapped");
        }
    }

    {
        const int block_count = get_resource_count(GL_SHADER_STORAGE_BLOCK);
        for(int i = 0; i != block_count; ++i) {
            const unsigned binding = get_resource_property(GL_SHADER_STORAGE_BLOCK, GL_BUFFER_BINDING, i);

            const int buffer = get_at(GL_SHADER_STORAGE_BUFFER_BINDING, binding);
            ALWAYS_ASSERT(buffer && glIsBuffer(buffer), "Bound storage buffer is destroyed or invalid");

            void* mapping = nullptr;
            glGetNamedBufferPointerv(buffer, GL_BUFFER_MAP_POINTER, &mapping);
            ALWAYS_ASSERT(!mapping, "Storage buffer is still mapped");
        }
    }
}

}
