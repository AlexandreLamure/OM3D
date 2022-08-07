#ifndef PROGRAM_H
#define PROGRAM_H

#include <graphics.h>

#include <vector>

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
        ~Program();

        void bind() const;

        static Program from_files(const std::string& frag, const std::string& vert);

        void set_uniform(u32 name_hash, float value);

        template<typename T>
        void set_uniform(std::string_view name, const T& value) {
            set_uniform(str_hash(name), value);
        }

    private:
        int find_location(u32 hash);

        GLHandle _handle;
        std::vector<UniformLocationInfo> _uniform_locations;

};

#endif // PROGRAM_H
