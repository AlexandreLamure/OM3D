
#include <array>
#include <glad/gl.h>

#include "glm/ext/matrix_float3x3.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "imgui/imgui_internal.h"

#define GLFW_INCLUDE_NONE
#include <Framebuffer.h>
#include <GLFW/glfw3.h>
#include <ImGuiRenderer.h>
#include <Scene.h>
#include <Texture.h>
#include <TimestampQuery.h>
#include <filesystem>
#include <graphics.h>
#include <imgui/imgui.h>
#include <iostream>
#include <shader_structs.h>
#include <vector>

using namespace OM3D;

static float delta_time = 0.0f;
static std::unique_ptr<Scene> scene;
static float exposure = 1.0;
static std::vector<std::string> scene_files;

namespace OM3D
{
    extern bool audit_bindings_before_draw;
}

void parse_args(int argc, char** argv)
{
    for (int i = 1; i < argc; ++i)
    {
        const std::string_view arg = argv[i];

        if (arg == "--validate")
        {
            OM3D::audit_bindings_before_draw = true;
        }
        else
        {
            std::cerr << "Unknown argument \"" << arg << "\"" << std::endl;
        }
    }
}

static bool in_plane(const glm::vec3& n, const glm::vec3& p,
                     const glm::vec3 center, float radius)
{
    glm::vec3 v = center + glm::normalize(n) * radius;
    glm::vec3 x = v - p;
    return glm::dot(x, n) >= 0;
}

void glfw_check(bool cond)
{
    if (!cond)
    {
        const char* err = nullptr;
        glfwGetError(&err);
        std::cerr << "GLFW error: " << err << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

void update_delta_time()
{
    static double time = 0.0;
    const double new_time = program_time();
    delta_time = float(new_time - time);
    time = new_time;
}

void process_inputs(GLFWwindow* window, Camera& camera)
{
    static glm::dvec2 mouse_pos;

    glm::dvec2 new_mouse_pos;
    glfwGetCursorPos(window, &new_mouse_pos.x, &new_mouse_pos.y);

    {
        glm::vec3 movement = {};
        if (glfwGetKey(window, 'W') == GLFW_PRESS)
        {
            movement += camera.forward();
        }
        if (glfwGetKey(window, 'S') == GLFW_PRESS)
        {
            movement -= camera.forward();
        }
        if (glfwGetKey(window, 'D') == GLFW_PRESS)
        {
            movement += camera.right();
        }
        if (glfwGetKey(window, 'A') == GLFW_PRESS)
        {
            movement -= camera.right();
        }

        float speed = 10.0f;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        {
            speed *= 10.0f;
        }

        if (movement.length() > 0.0f)
        {
            const glm::vec3 new_pos =
                camera.position() + movement * delta_time * speed;
            camera.set_view(
                glm::lookAt(new_pos, new_pos + camera.forward(), camera.up()));
        }
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        const glm::vec2 delta = glm::vec2(mouse_pos - new_mouse_pos) * 0.01f;
        if (delta.length() > 0.0f)
        {
            glm::mat4 rot = glm::rotate(glm::mat4(1.0f), delta.x,
                                        glm::vec3(0.0f, 1.0f, 0.0f));
            rot = glm::rotate(rot, delta.y, camera.right());
            camera.set_view(glm::lookAt(
                camera.position(),
                camera.position() + (glm::mat3(rot) * camera.forward()),
                (glm::mat3(rot) * camera.up())));
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

void gui(ImGuiRenderer& imgui)
{
    const ImVec4 error_text_color = ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
    const ImVec4 warning_text_color = ImVec4(1.0f, 0.8f, 0.4f, 1.0f);

    static bool open_gpu_profiler = false;

    PROFILE_GPU("GUI");

    imgui.start();
    DEFER(imgui.finish());

    // ImGui::ShowDemoWindow();

    bool open_scene_popup = false;
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open Scene"))
            {
                open_scene_popup = true;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Exposure"))
        {
            ImGui::DragFloat("Exposure", &exposure, 0.25f, 0.01f, 100.0f,
                             "%.2f", ImGuiSliderFlags_Logarithmic);
            if (exposure != 1.0f && ImGui::Button("Reset"))
            {
                exposure = 1.0f;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Debug"))
        {
            ImGui::PushItemFlag(ImGuiItemFlags_SelectableDontClosePopup, true);
            if (ImGui::Selectable("None", imgui._debug_texture == 3))
                imgui._debug_texture = 3;
            if (ImGui::Selectable("Albedo", imgui._debug_texture == 0))
                imgui._debug_texture = 0;
            if (ImGui::Selectable("Normal", imgui._debug_texture == 1))
                imgui._debug_texture = 1;
            if (ImGui::Selectable("Depth", imgui._debug_texture == 2))
                imgui._debug_texture = 2;
            ImGui::PopItemFlag();
            ImGui::EndMenu();
        }

        if (scene && ImGui::BeginMenu("Scene Info"))
        {
            ImGui::Text("%u objects", u32(scene->objects().size()));
            ImGui::Text("%u point lights", u32(scene->point_lights().size()));
            ImGui::EndMenu();
        }

        if (ImGui::MenuItem("GPU Profiler"))
        {
            open_gpu_profiler = true;
        }

        ImGui::Separator();
        ImGui::TextUnformatted(
            reinterpret_cast<const char*>(glGetString(GL_RENDERER)));

        ImGui::Separator();
        ImGui::Text("%.2f ms", delta_time * 1000.0f);

#ifdef OM3D_DEBUG
        ImGui::Separator();
        ImGui::TextColored(warning_text_color, ICON_FA_BUG " (DEBUG)");
#endif

        if (!bindless_enabled())
        {
            ImGui::Separator();
            ImGui::TextColored(error_text_color,
                               ICON_FA_EXCLAMATION_TRIANGLE
                               " Bindless textures not supported");
        }
        ImGui::EndMainMenuBar();
    }

    if (open_scene_popup)
    {
        ImGui::OpenPopup("###openscenepopup");

        scene_files.clear();
        for (auto&& entry : std::filesystem::directory_iterator(data_path))
        {
            if (entry.status().type() == std::filesystem::file_type::regular)
            {
                const auto ext = entry.path().extension();
                if (ext == ".gltf" || ext == ".glb")
                {
                    scene_files.emplace_back(entry.path().string());
                }
            }
        }
    }

    if (ImGui::BeginPopup("###openscenepopup",
                          ImGuiWindowFlags_AlwaysAutoResize))
    {
        auto load_scene = [](const std::string path) {
            auto result = Scene::from_gltf(path);
            if (!result.is_ok)
            {
                std::cerr << "Unable to load scene (" << path << ")"
                          << std::endl;
            }
            else
            {
                scene = std::move(result.value);
            }
            ImGui::CloseCurrentPopup();
        };

        char buffer[1024] = {};
        if (ImGui::InputText("Load scene", buffer, sizeof(buffer),
                             ImGuiInputTextFlags_EnterReturnsTrue))
        {
            load_scene(buffer);
        }

        if (!scene_files.empty())
        {
            for (const std::string& p : scene_files)
            {
                const auto abs = std::filesystem::absolute(p).string();
                if (ImGui::MenuItem(abs.c_str()))
                {
                    load_scene(p);
                    break;
                }
            }
        }

        ImGui::EndPopup();
    }

    if (open_gpu_profiler)
    {
        if (ImGui::Begin(ICON_FA_CLOCK " GPU Profiler"))
        {
            const ImGuiTableFlags table_flags = ImGuiTableFlags_SortTristate
                | ImGuiTableFlags_NoSavedSettings
                | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersInnerV
                | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg;

            ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt,
                                  ImVec4(1, 1, 1, 0.01f));
            DEFER(ImGui::PopStyleColor());

            if (ImGui::BeginTable("##timetable", 3, table_flags))
            {
                ImGui::TableSetupColumn("Name",
                                        ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("CPU (ms)",
                                        ImGuiTableColumnFlags_NoResize, 70.0f);
                ImGui::TableSetupColumn("GPU (ms)",
                                        ImGuiTableColumnFlags_NoResize, 70.0f);
                ImGui::TableHeadersRow();

                std::vector<u32> indents;
                for (const auto& zone : retrieve_profile())
                {
                    auto color_from_time = [](float time) {
                        const float t =
                            std::min(time / 0.008f, 1.0f); // 8ms = red
                        return ImVec4(t, 1.0f - t, 0.0f, 1.0f);
                    };

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted(zone.name.data());

                    ImGui::TableSetColumnIndex(1);
                    ImGui::PushStyleColor(ImGuiCol_Text,
                                          color_from_time(zone.cpu_time));
                    ImGui::Text("%.2f", zone.cpu_time * 1000.0f);

                    ImGui::TableSetColumnIndex(2);
                    ImGui::PushStyleColor(ImGuiCol_Text,
                                          color_from_time(zone.gpu_time));
                    ImGui::Text("%.2f", zone.gpu_time * 1000.0f);

                    ImGui::PopStyleColor(2);

                    if (!indents.empty() && --indents.back() == 0)
                    {
                        indents.pop_back();
                        ImGui::Unindent();
                    }

                    if (zone.contained_zones)
                    {
                        indents.push_back(zone.contained_zones);
                        ImGui::Indent();
                    }
                }

                ImGui::EndTable();
            }
        }
        ImGui::End();
    }
}

std::unique_ptr<Scene> create_default_scene()
{
    auto scene = std::make_unique<Scene>();

    // Load default cube model
    auto result = Scene::from_gltf(std::string(data_path) + "cube.glb");
    ALWAYS_ASSERT(result.is_ok, "Unable to load default scene");
    scene = std::move(result.value);

    auto sphere = Scene::from_gltf(std::string(data_path) + "sphere.glb");
    scene->add_object(sphere.value->objects()[0]);

    scene->set_sun(glm::vec3(0.2f, 1.0f, 0.1f), glm::vec3(1.0f));

    // Add lights
    {
        PointLight light;
        light.set_position(glm::vec3(1.0f, 2.0f, 4.0f));
        // light.set_color(glm::vec3(0.0f, 0.0f, 50.0f));
        light.set_color(glm::vec3(0.0f, 50.0f, 0.0f));
        light.set_radius(100.0f);
        // light.set_radius(2.0f);
        scene->add_light(std::move(light));
    }
    {
        PointLight light;
        light.set_position(glm::vec3(1.0f, 2.0f, -4.0f));
        light.set_color(glm::vec3(50.0f, 0.0f, 0.0f));
        // light.set_radius(4.0f);
        light.set_radius(50.0f);
        scene->add_light(std::move(light));
    }

    return scene;
}

struct RendererState
{
    static RendererState create(glm::uvec2 size)
    {
        RendererState state;

        state.size = size;

        if (state.size.x > 0 && state.size.y > 0)
        {
            state.depth_texture = Texture(size, ImageFormat::Depth32_FLOAT);
            state.lit_hdr_texture = Texture(size, ImageFormat::RGBA16_FLOAT);
            state.tone_mapped_texture = Texture(size, ImageFormat::RGBA8_UNORM);
            state.g_albedo_texture = Texture(size, ImageFormat::RGBA8_sRGB);
            state.g_normal_texture = Texture(size, ImageFormat::RGBA8_UNORM);
            state.g_debug_texture = Texture(size, ImageFormat::RGBA8_UNORM);
            state.g_illum_texture = Texture(size, ImageFormat::RGBA8_UNORM);
            state.main_framebuffer = Framebuffer(
                &state.depth_texture, std::array{ &state.lit_hdr_texture });
            state.tone_map_framebuffer =
                Framebuffer(nullptr, std::array{ &state.tone_mapped_texture });
            state.z_prepass_framebuffer = Framebuffer(&state.depth_texture);
            state.g_framebuffer = Framebuffer(
                &state.depth_texture,
                std::array{ &state.g_albedo_texture, &state.g_normal_texture });
            state.g_debug_framebuffer =
                Framebuffer(nullptr, std::array{ &state.g_debug_texture });
            state.g_illumination_framebuffer =
                Framebuffer(nullptr, std::array{ &state.g_illum_texture });
        }

        return state;
    }

    glm::uvec2 size = {};

    Texture depth_texture;
    Texture lit_hdr_texture;
    Texture tone_mapped_texture;

    Framebuffer main_framebuffer;
    Framebuffer tone_map_framebuffer;

    // Z Prepass
    Framebuffer z_prepass_framebuffer;

    // G Buffer
    Framebuffer g_framebuffer;
    Texture g_albedo_texture;
    Texture g_normal_texture;

    Framebuffer g_debug_framebuffer;
    Texture g_debug_texture;

    Framebuffer g_illumination_framebuffer;
    Texture g_illum_texture;
};

int main(int argc, char** argv)
{
    DEBUG_ASSERT([] {
        std::cout << "Debug asserts enabled" << std::endl;
        return true;
    }());

    parse_args(argc, argv);

    glfw_check(glfwInit());
    DEFER(glfwTerminate());

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1600, 900, "OM3D", nullptr, nullptr);
    glfw_check(window);
    DEFER(glfwDestroyWindow(window));

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    init_graphics();

    ImGuiRenderer imgui(window);

    scene = create_default_scene();

    std::cout << "Scene has " << scene->objects().size() << " objects\n";

    auto tonemap_program = Program::from_files("tonemap.frag", "screen.vert");
    auto g_debug_program = Program::from_files("g_debug.frag", "screen.vert");
    auto g_global_illumination_program =
        Program::from_files("g_global_illumination.frag", "screen.vert");
    auto g_local_illumination_program =
        Program::from_files("g_local_illumination.frag", "basic.vert");

    auto light_material = Material::empty_material();
    light_material->set_program(g_local_illumination_program);
    light_material->set_blend_mode(BlendMode::Additive);
    light_material->set_depth_test_mode(DepthTestMode::Reversed);
    auto sphere = scene->objects()[1];
    sphere.set_material(light_material);

    RendererState renderer;

    for (size_t i = 0; i < scene->point_lights().size(); i++)
    {
        const auto light = scene->point_lights()[i];
        const auto position = light.position();
        std::cout << position.x << ", " << position.y << ", " << position.z
                  << ", " << light.radius() << std::endl;
    }

    for (;;)
    {
        glfwPollEvents();
        if (glfwWindowShouldClose(window)
            || glfwGetKey(window, GLFW_KEY_ESCAPE))
        {
            break;
        }

        process_profile_markers();

        {
            int width = 0;
            int height = 0;
            glfwGetWindowSize(window, &width, &height);

            if (renderer.size != glm::uvec2(width, height))
            {
                renderer = RendererState::create(glm::uvec2(width, height));
            }
        }

        update_delta_time();

        if (const auto& io = ImGui::GetIO();
            !io.WantCaptureMouse && !io.WantCaptureKeyboard)
        {
            process_inputs(window, scene->camera());
        }

        // Draw everything
        {
            PROFILE_GPU("Frame");

            // Z prepass
            {
                PROFILE_GPU("Z pass");
                renderer.z_prepass_framebuffer.bind(true, false);
                scene->render();
            }

            // Render the scene
            {
                PROFILE_GPU("Main pass");

                renderer.g_framebuffer.bind(false, true);
                scene->render();
            }

            if (imgui._debug_texture != 3)
            {
                PROFILE_GPU("G buffer debug");

                glDisable(GL_CULL_FACE);

                renderer.g_debug_framebuffer.bind(true, true);

                g_debug_program->bind();
                g_debug_program->set_uniform(HASH("texture"),
                                             imgui._debug_texture);
                renderer.g_albedo_texture.bind(0);
                renderer.g_normal_texture.bind(1);
                renderer.depth_texture.bind(2);
                glDrawArrays(GL_TRIANGLES, 0, 3);
            }
            else
            {
                {
                    PROFILE_GPU("Global Illumination");

                    glDisable(GL_CULL_FACE);
                    renderer.g_debug_framebuffer.bind(true, true);

                    g_global_illumination_program->bind();
                    TypedBuffer<shader::FrameData> buffer(nullptr, 1);
                    {
                        auto mapping = buffer.map(AccessType::WriteOnly);
                        mapping[0].camera.view_proj = scene->view_proj_matrix();
                        mapping[0].point_light_count =
                            scene->point_lights().size();
                        mapping[0].sun_color = scene->get_sun_color();
                        mapping[0].sun_dir = scene->get_sun_direction();
                    }
                    buffer.bind(BufferUsage::Uniform, 0);

                    renderer.g_albedo_texture.bind(0);
                    renderer.g_normal_texture.bind(1);
                    renderer.depth_texture.bind(2);
                    glDrawArrays(GL_TRIANGLES, 0, 3);
                }
                {
                    PROFILE_GPU("Local Illumination");

                    glDisable(GL_CULL_FACE);
                    renderer.g_debug_framebuffer.bind(false, false);
                    light_material->bind(false);

                    // Fill and bind lights buffer
                    TypedBuffer<shader::FrameData> buffer(nullptr, 1);
                    {
                        auto mapping = buffer.map(AccessType::WriteOnly);
                        mapping[0].camera.view_proj = scene->view_proj_matrix();
                        mapping[0].point_light_count =
                            scene->point_lights().size();
                    }
                    buffer.bind(BufferUsage::Uniform, 0);

                    TypedBuffer<shader::PointLight> light_buffer(
                        nullptr,
                        std::max(scene->point_lights().size(), size_t(1)));
                    {
                        auto mapping = light_buffer.map(AccessType::WriteOnly);
                        for (size_t i = 0; i != scene->point_lights().size();
                             ++i)
                        {
                            const auto light = scene->point_lights()[i];
                            mapping[i] = { light.position(), light.radius(),
                                           light.color(), 0.0f };
                        }
                    }
                    light_buffer.bind(BufferUsage::Storage, 1);

                    renderer.g_albedo_texture.bind(0);
                    renderer.g_normal_texture.bind(1);
                    renderer.depth_texture.bind(2);

                    const auto& camera = scene->camera();
                    const auto& frustum = camera.build_frustum();
                    for (size_t i = 0; i < scene->point_lights().size(); i++)
                    {
                        const auto light = scene->point_lights()[i];

                        bool to_draw =
                            in_plane(frustum._left_normal, camera.position(),
                                     light.position(), light.radius());
                        to_draw &=
                            in_plane(frustum._top_normal, camera.position(),
                                     light.position(), light.radius());
                        to_draw &=
                            in_plane(frustum._right_normal, camera.position(),
                                     light.position(), light.radius());
                        to_draw &=
                            in_plane(frustum._bottom_normal, camera.position(),
                                     light.position(), light.radius());
                        to_draw &=
                            in_plane(frustum._near_normal, camera.position(),
                                     light.position(), light.radius());

                        if (!to_draw)
                            continue;

                        light_material->set_uniform(HASH("light_id"),
                                                   static_cast<OM3D::u32>(i));

                        sphere.set_transform(glm::translate(glm::mat4(1.0f), light.position()) *
                                             glm::scale(glm::mat4(1.0f), glm::vec3(light.radius())));

                        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                        sphere.render(camera, frustum);
                        // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                        // glDrawArrays(GL_TRIANGLES, 0, 3);
                    }
                }
            }

            // Apply a tonemap in compute shader
            {
                // PROFILE_GPU("Tonemap");
                //
                // glDisable(GL_CULL_FACE);
                // renderer.tone_map_framebuffer.bind(false, true);
                // tonemap_program->bind();
                // tonemap_program->set_uniform(HASH("exposure"), exposure);
                // renderer.g_debug_texture.bind(0);
                // glDrawArrays(GL_TRIANGLES, 0, 3);
            }

            // Blit tonemap result to screen
            {
                PROFILE_GPU("Blit");

                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                renderer.g_debug_framebuffer.blit();
            }

            // Draw GUI on top
            gui(imgui);
        }

        glfwSwapBuffers(window);
    }

    scene = nullptr; // destroy scene and child OpenGL objects
}
