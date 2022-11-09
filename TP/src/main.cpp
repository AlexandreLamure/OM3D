#include <glad/glad.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>

#include <graphics.h>
#include <SceneView.h>
#include <Texture.h>
#include <Framebuffer.h>
#include <ImGuiRenderer.h>

#include <imgui/imgui.h>


using namespace OM3D;

static float dt = 0.0f;
const glm::uvec2 window_size(1600, 900);


void glfw_check(bool cond) {
    if(!cond) {
        const char* err = nullptr;
        glfwGetError(&err);
        std::cerr << "GLFW error: " << err << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

void update_dt() {
    static double time = 0.0;
    const double new_time = program_time();
    dt = float(new_time - time);
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
        const float speed = 10.0f;
        if(movement.length() > 0.0f) {
            const glm::vec3 new_pos = camera.position() + movement * dt * speed;
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

    mouse_pos = new_mouse_pos;
}


std::unique_ptr<Scene> create_scene() {
    auto scene = std::make_unique<Scene>();

    {
        const auto r = MeshData::from_obj(std::string(data_path) + "cube.obj");
        ALWAYS_ASSERT(r.is_ok, "Unable to load mesh");
        std::shared_ptr<StaticMesh> mesh = std::make_shared<StaticMesh>(r.value);
        std::shared_ptr<Material> material = std::make_shared<Material>();
        material->set_program(Program::from_files("lit.frag", "basic.vert", {"TEXTURED"}));
        {
            const auto r = TextureData::from_file(std::string(data_path) + "uv.png");
            ALWAYS_ASSERT(r.is_ok, "Unable to load texture");
            material->set_texture(0u, std::make_shared<Texture>(r.value));
        }
        scene->add_object(SceneObject(std::move(mesh), std::move(material)));
    }

    {
        PointLight light;
        light.set_position(glm::vec3(1.0f, 2.0f, 4.0f));
        light.set_color(glm::vec3(0.0f, 10.0f, 0.0f));
        light.set_radius(100.0f);
        scene->add_object(std::move(light));
    }
    {
        PointLight light;
        light.set_position(glm::vec3(1.0f, 2.0f, -4.0f));
        light.set_color(glm::vec3(10.0f, 0.0f, 0.0f));
        light.set_radius(50.0f);
        scene->add_object(std::move(light));
    }

    return scene;
}


int main(int, char**) {
    DEBUG_ASSERT([] { std::cout << "Debug asserts enabled" << std::endl; return true; }());

    glfw_check(glfwInit());
    DEFER(glfwTerminate());

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(window_size.x, window_size.y, "TP window", nullptr, nullptr);
    glfw_check(window);
    DEFER(glfwDestroyWindow(window));

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    init_graphics();

    ImGuiRenderer imgui(window);

    std::unique_ptr<Scene> scene = create_scene();
    SceneView scene_view(scene.get());

    auto fps_program = Program::from_files("fps.frag", "screen.vert");
    auto tonemap_program = Program::from_file("tonemap.comp");

    Texture depth(window_size, ImageFormat::Depth32_FLOAT);
    Texture lit(window_size, ImageFormat::RGBA16_FLOAT);
    Texture color(window_size, ImageFormat::RGBA8_UNORM);
    Framebuffer main_framebuffer(&depth, std::array{&lit});
    Framebuffer tonemap_framebuffer(nullptr, std::array{&color});

    for(;;) {
        glfwPollEvents();
        if(glfwWindowShouldClose(window) || glfwGetKey(window, GLFW_KEY_ESCAPE)) {
            break;
        }

        update_dt();

        if(const auto& io = ImGui::GetIO(); !io.WantCaptureMouse && !io.WantCaptureKeyboard) {
            process_inputs(window, scene_view.camera());
        }

        {
            main_framebuffer.bind();
            scene_view.render();
        }

        {
            tonemap_program->bind();
            lit.bind(0);
            color.bind_as_image(1, AccessType::WriteOnly);
            glDispatchCompute(align_up_to(window_size.x, 8), align_up_to(window_size.y, 8), 1);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        tonemap_framebuffer.blit();

        /*{
            glDisable(GL_DEPTH_TEST);
            fps_program->set_uniform(HASH("delta_time"), dt);
            fps_program->bind();
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }*/

        imgui.start();
        {
            char buffer[1024] = {};
            if(ImGui::InputText("Load scene", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
                auto result = Scene::from_gltf(buffer);
                if(!result.is_ok) {
                    std::cerr << "Unable to load scene (" << buffer << ")" << std::endl;
                } else {
                    scene = std::move(result.value);
                    scene_view = SceneView(scene.get());
                }
            }
        }
        imgui.finish();

        glfwSwapBuffers(window);
    }
}
