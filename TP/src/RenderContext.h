#ifndef RENDERCONTEXT_H
#define RENDERCONTEXT_H

#include <Program.h>

#include <string_view>

class RenderContext : NonMovable {
    public:
        static constexpr std::string_view shader_path = "../../shaders/";

        RenderContext();
        ~RenderContext();

        void draw_screen();

    private:
        Program _screen;
};

#endif // RENDERCONTEXT_H
