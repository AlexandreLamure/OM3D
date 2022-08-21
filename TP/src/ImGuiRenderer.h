#ifndef IMGUIRENDERER_H
#define IMGUIRENDERER_H

#include <Texture.h>
#include <Program.h>

#include <chrono>

struct ImDrawData;
struct GLFWwindow;

class ImGuiRenderer : NonMovable {
    public:
        ImGuiRenderer();

        void render(GLFWwindow* window);

    private:
        void render(const ImDrawData* draw_data);
        float update_dt();

        Program _program;
        std::unique_ptr<Texture> _font;
        std::chrono::time_point<std::chrono::high_resolution_clock> _last;
};

#endif // IMGUIRENDERER_H
