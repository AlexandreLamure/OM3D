#ifndef PROGRAM_H
#define PROGRAM_H

#include <graphics.h>

class Program : NonCopyable {
    public:
        Program() = default;
        Program(Program&&) = default;
        Program& operator=(Program&&) = default;

        Program(const std::string& frag, const std::string& vert);
        ~Program();

        static Program from_files(const std::string& frag, const std::string& vert);

    private:
        GLHandle _handle;
};

#endif // PROGRAM_H
