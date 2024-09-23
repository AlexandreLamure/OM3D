#ifndef IMGUIRENDERER_H
#define IMGUIRENDERER_H

#include <Material.h>

#include <chrono>

#include <imgui/IconsFontAwesome5.h>

struct ImDrawData;
struct GLFWwindow;

namespace OM3D {

class ImGuiRenderer : NonMovable {
    public:
        ImGuiRenderer(GLFWwindow* window);

        void start();
        void finish();

    private:
        void render(const ImDrawData* draw_data);
        float update_delta_time();

        GLFWwindow* _window = nullptr;

        Material _material;
        std::unique_ptr<Texture> _font;
        std::chrono::time_point<std::chrono::high_resolution_clock> _last;
};

}

#endif // IMGUIRENDERER_H
