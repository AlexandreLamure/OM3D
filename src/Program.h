#ifndef PROGRAM_H
#define PROGRAM_H

#include <graphics.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat2x2.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>

#include <memory>
#include <vector>

namespace OM3D {

class Program : NonCopyable {

    struct UniformLocationInfo {
        u32 name_hash;
        int location;

        bool operator<(const UniformLocationInfo& other) const {
            return name_hash < other.name_hash;
        }

        bool operator==(const UniformLocationInfo& other) const {
            return name_hash == other.name_hash;
        }
    };

    public:
        Program() = default;
        Program(Program&&) = default;
        Program& operator=(Program&&) = default;

        Program(const std::string& frag, const std::string& vert);
        Program(const std::string& comp);
        ~Program();

        void bind() const;

        bool is_compute() const;

        static std::shared_ptr<Program> from_file(const std::string& comp, Span<const std::string> defines = {});
        static std::shared_ptr<Program> from_files(const std::string& frag, const std::string& vert, Span<const std::string> defines = {});

        void set_uniform(u32 name_hash, u32 value);
        void set_uniform(u32 name_hash, float value);
        void set_uniform(u32 name_hash, glm::vec2 value);
        void set_uniform(u32 name_hash, glm::vec3 value);
        void set_uniform(u32 name_hash, glm::vec4 value);
        void set_uniform(u32 name_hash, const glm::mat2& value);
        void set_uniform(u32 name_hash, const glm::mat3& value);
        void set_uniform(u32 name_hash, const glm::mat4& value);
        void set_uniform(u32 name_hash, u64 value);

        template<typename T>
        void set_uniform(std::string_view name, const T& value) {
            set_uniform(str_hash(name), value);
        }

    private:
        void fetch_uniform_locations();
        int find_location(u32 hash);

        GLHandle _handle;
        std::vector<UniformLocationInfo> _uniform_locations;

        bool _is_compute = false;

};

}

#endif // PROGRAM_H
