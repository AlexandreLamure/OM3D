#ifndef PROGRAM_H
#define PROGRAM_H

#include <utils.h>

class Program : NonCopyable {
    public:
        Program() = default;
        Program(Program&&) = default;
        Program& operator=(Program&&) = default;

        Program(const std::string& frag, const std::string& vert);
        ~Program();

    private:
        friend class RenderContext;

        GLHandle _handle;
};

#endif // PROGRAM_H
