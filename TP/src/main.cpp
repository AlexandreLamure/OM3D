#include <glad/glad.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <graphics.h>
#include <SceneView.h>
#include <Texture.h>
#include <Framebuffer.h>
#include <ImGuiRenderer.h>

#include <imgui/imgui.h>

#include <iostream>
#include <vector>
#include <filesystem>

using namespace OM3D;

static float delta_time = 0.0f;
static std::unique_ptr<Scene> scene;
static SceneView scene_view;
static float exposure = 1.0;
static std::vector<std::string> scene_files;




void glfw_check(bool cond) {
    if(!cond) {
        const char* err = nullptr;
        glfwGetError(&err);
        std::cerr << "GLFW error: " << err << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

void update_delta_time() {
    static double time = 0.0;
    const double new_time = program_time();
    delta_time = float(new_time - time);
    time = new_time;
}

void process_inputs(GLFWwindow* window, Camera& camera) {
    static glm::dvec2 mouse_pos;

    glm::dvec2 new_mouse_pos;
    glfwGetCursorPos(window, &new_mouse_pos.x, &new_mouse_pos.y);

    {
        glm::vec3 movement = {};
        if(glfwGetKey(window, 'W') == GLFW_PRESS) {
            movement += camera.forward();
        }
        if(glfwGetKey(window, 'S') == GLFW_PRESS) {
            movement -= camera.forward();
        }
        if(glfwGetKey(window, 'D') == GLFW_PRESS) {
            movement += camera.right();
        }
        if(glfwGetKey(window, 'A') == GLFW_PRESS) {
            movement -= camera.right();
        }

        float speed = 10.0f;
        if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            speed *= 10.0f;
        }

        if(movement.length() > 0.0f) {
            const glm::vec3 new_pos = camera.position() + movement * delta_time * speed;
            camera.set_view(glm::lookAt(new_pos, new_pos + camera.forward(), camera.up()));
        }
    }

    if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        const glm::vec2 delta = glm::vec2(mouse_pos - new_mouse_pos) * 0.01f;
        if(delta.length() > 0.0f) {
            glm::mat4 rot = glm::rotate(glm::mat4(1.0f), delta.x, glm::vec3(0.0f, 1.0f, 0.0f));
            rot = glm::rotate(rot, delta.y, camera.right());
            camera.set_view(glm::lookAt(camera.position(), camera.position() + (glm::mat3(rot) * camera.forward()), (glm::mat3(rot) * camera.up())));
        }
    }

    {
        int width = 0;
        int height = 0;
        glfwGetWindowSize(window, &width, &height);
        camera.set_ratio(float(width) / float(height));
    }

    mouse_pos = new_mouse_pos;
}

void gui(ImGuiRenderer& imgui) {
    imgui.start();
    DEFER(imgui.finish());

    // ImGui::ShowDemoWindow();

    bool open_scene_popup = false;
    if(ImGui::BeginMainMenuBar()) {
        if(ImGui::BeginMenu("File")) {
            if(ImGui::MenuItem("Open Scene")) {
                open_scene_popup = true;
            }
            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("Exposure")) {
            ImGui::DragFloat("Exposure", &exposure, 0.25f, 0.01f, 100.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
            if(exposure != 1.0f && ImGui::Button("Reset")) {
                exposure = 1.0f;
            }
            ImGui::EndMenu();
        }

        ImGui::Separator();

        ImGui::Text("%.2f ms", delta_time * 1000.0f);

        ImGui::EndMainMenuBar();
    }

    if(open_scene_popup) {
        ImGui::OpenPopup("###openscenepopup");

        for(auto&& entry : std::filesystem::directory_iterator(data_path)) {
            if(entry.status().type() == std::filesystem::file_type::regular) {
                const auto ext = entry.path().extension();
                if(ext == ".gltf" || ext == ".glb") {
                    scene_files.emplace_back(entry.path().string());
                }
            }
        }
    }

    if(ImGui::BeginPopup("###openscenepopup", ImGuiWindowFlags_AlwaysAutoResize)) {
        auto load_scene = [](const std::string path) {
            auto result = Scene::from_gltf(path);
            if(!result.is_ok) {
                std::cerr << "Unable to load scene (" << path << ")" << std::endl;
            } else {
                scene = std::move(result.value);
                scene_view = SceneView(scene.get());
            }
            ImGui::CloseCurrentPopup();
        };

        char buffer[1024] = {};
        if(ImGui::InputText("Load scene", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
            load_scene(buffer);
        }

        if(!scene_files.empty()) {
            for(const std::string& p : scene_files) {
                const auto abs = std::filesystem::absolute(p).string();
                if(ImGui::MenuItem(abs.c_str())) {
                    load_scene(p);
                    break;
                }
            }
        }

        ImGui::EndPopup();
    }
}




std::unique_ptr<Scene> create_default_scene() {
    auto scene = std::make_unique<Scene>();

    // Load default cube model
    auto result = Scene::from_gltf(std::string(data_path) + "cube.glb");
    ALWAYS_ASSERT(result.is_ok, "Unable to load default scene");
    scene = std::move(result.value);

    scene->set_sun(glm::vec3(0.2f, 1.0f, 0.1f), glm::vec3(1.0f));

    // Add lights
    {
        PointLight light;
        light.set_position(glm::vec3(1.0f, 2.0f, 4.0f));
        light.set_color(glm::vec3(0.0f, 50.0f, 0.0f));
        light.set_radius(100.0f);
        scene->add_object(std::move(light));
    }
    {
        PointLight light;
        light.set_position(glm::vec3(1.0f, 2.0f, -4.0f));
        light.set_color(glm::vec3(50.0f, 0.0f, 0.0f));
        light.set_radius(50.0f);
        scene->add_object(std::move(light));
    }

    return scene;
}

struct RendererState {
    static RendererState create(glm::uvec2 size) {
        RendererState state;

        state.size = size;

        if(state.size.x > 0 && state.size.y > 0) {
            state.depth_texture = Texture(size, ImageFormat::Depth32_FLOAT);
            state.lit_hdr_texture = Texture(size, ImageFormat::RGBA16_FLOAT);
            state.tone_mapped_texture = Texture(size, ImageFormat::RGBA8_UNORM);
            state.main_framebuffer = Framebuffer(&state.depth_texture, std::array{&state.lit_hdr_texture});
            state.tone_map_framebuffer = Framebuffer(nullptr, std::array{&state.tone_mapped_texture});
        }

        return state;
    }

    glm::uvec2 size = {};

    Texture depth_texture;
    Texture lit_hdr_texture;
    Texture tone_mapped_texture;

    Framebuffer main_framebuffer;
    Framebuffer tone_map_framebuffer;
};


int main(int, char**) {
    DEBUG_ASSERT([] { std::cout << "Debug asserts enabled" << std::endl; return true; }());

    glfw_check(glfwInit());
    DEFER(glfwTerminate());

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    GLFWwindow* window = glfwCreateWindow(1600, 900, "TP window", nullptr, nullptr);
    glfw_check(window);
    DEFER(glfwDestroyWindow(window));

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    init_graphics();

    ImGuiRenderer imgui(window);

    scene = create_default_scene();
    scene_view = SceneView(scene.get());

    auto tonemap_program = Program::from_files("tonemap.frag", "screen.vert");
    RendererState renderer;

    for(;;) {

        glfwPollEvents();
        if(glfwWindowShouldClose(window) || glfwGetKey(window, GLFW_KEY_ESCAPE)) {
            break;
        }

        {
            int width = 0;
            int height = 0;
            glfwGetWindowSize(window, &width, &height);

            if(renderer.size != glm::uvec2(width, height)) {
                renderer = RendererState::create(glm::uvec2(width, height));
            }
        }

        update_delta_time();

        if(const auto& io = ImGui::GetIO(); !io.WantCaptureMouse && !io.WantCaptureKeyboard) {
            process_inputs(window, scene_view.camera());
        }

        // Render the scene
        {
            renderer.main_framebuffer.bind();
            scene_view.render();
        }

        // Apply a tonemap in compute shader
        {
            renderer.tone_map_framebuffer.bind();
            tonemap_program->bind();
            tonemap_program->set_uniform(HASH("exposure"), exposure);
            renderer.lit_hdr_texture.bind(0);
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }

        // Blit tonemap result to screen
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        renderer.tone_map_framebuffer.blit();

        gui(imgui);

        glfwSwapBuffers(window);
    }

    scene = nullptr; // destroy scene and child OpenGL objects
}
