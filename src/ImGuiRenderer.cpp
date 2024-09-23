#include "ImGuiRenderer.h"

#include <TypedBuffer.h>

#include <glm/vec2.hpp>

#include <imgui/imgui.h>
#include <glad/gl.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <algorithm>

#include <imgui/fa-solid-900.h>


namespace OM3D {

static ImGuiMouseButton button_to_imgui(int button) {
    switch(button) {
        case GLFW_MOUSE_BUTTON_LEFT: return ImGuiMouseButton_Left;
        case GLFW_MOUSE_BUTTON_RIGHT: return ImGuiMouseButton_Right;
        case GLFW_MOUSE_BUTTON_MIDDLE: return ImGuiMouseButton_Middle;
        default:
            return ImGuiMouseButton_COUNT;
    }
}

// https://github.com/ocornut/imgui/blob/master/backends/imgui_impl_glfw.cpp#L137
static ImGuiKey key_to_imgui(int key) {
    switch(key) {
        case GLFW_KEY_TAB: return ImGuiKey_Tab;
        case GLFW_KEY_LEFT: return ImGuiKey_LeftArrow;
        case GLFW_KEY_RIGHT: return ImGuiKey_RightArrow;
        case GLFW_KEY_UP: return ImGuiKey_UpArrow;
        case GLFW_KEY_DOWN: return ImGuiKey_DownArrow;
        case GLFW_KEY_PAGE_UP: return ImGuiKey_PageUp;
        case GLFW_KEY_PAGE_DOWN: return ImGuiKey_PageDown;
        case GLFW_KEY_HOME: return ImGuiKey_Home;
        case GLFW_KEY_END: return ImGuiKey_End;
        case GLFW_KEY_INSERT: return ImGuiKey_Insert;
        case GLFW_KEY_DELETE: return ImGuiKey_Delete;
        case GLFW_KEY_BACKSPACE: return ImGuiKey_Backspace;
        case GLFW_KEY_SPACE: return ImGuiKey_Space;
        case GLFW_KEY_ENTER: return ImGuiKey_Enter;
        case GLFW_KEY_ESCAPE: return ImGuiKey_Escape;
        case GLFW_KEY_APOSTROPHE: return ImGuiKey_Apostrophe;
        case GLFW_KEY_COMMA: return ImGuiKey_Comma;
        case GLFW_KEY_MINUS: return ImGuiKey_Minus;
        case GLFW_KEY_PERIOD: return ImGuiKey_Period;
        case GLFW_KEY_SLASH: return ImGuiKey_Slash;
        case GLFW_KEY_SEMICOLON: return ImGuiKey_Semicolon;
        case GLFW_KEY_EQUAL: return ImGuiKey_Equal;
        case GLFW_KEY_LEFT_BRACKET: return ImGuiKey_LeftBracket;
        case GLFW_KEY_BACKSLASH: return ImGuiKey_Backslash;
        case GLFW_KEY_RIGHT_BRACKET: return ImGuiKey_RightBracket;
        case GLFW_KEY_GRAVE_ACCENT: return ImGuiKey_GraveAccent;
        case GLFW_KEY_CAPS_LOCK: return ImGuiKey_CapsLock;
        case GLFW_KEY_SCROLL_LOCK: return ImGuiKey_ScrollLock;
        case GLFW_KEY_NUM_LOCK: return ImGuiKey_NumLock;
        case GLFW_KEY_PRINT_SCREEN: return ImGuiKey_PrintScreen;
        case GLFW_KEY_PAUSE: return ImGuiKey_Pause;
        case GLFW_KEY_KP_0: return ImGuiKey_Keypad0;
        case GLFW_KEY_KP_1: return ImGuiKey_Keypad1;
        case GLFW_KEY_KP_2: return ImGuiKey_Keypad2;
        case GLFW_KEY_KP_3: return ImGuiKey_Keypad3;
        case GLFW_KEY_KP_4: return ImGuiKey_Keypad4;
        case GLFW_KEY_KP_5: return ImGuiKey_Keypad5;
        case GLFW_KEY_KP_6: return ImGuiKey_Keypad6;
        case GLFW_KEY_KP_7: return ImGuiKey_Keypad7;
        case GLFW_KEY_KP_8: return ImGuiKey_Keypad8;
        case GLFW_KEY_KP_9: return ImGuiKey_Keypad9;
        case GLFW_KEY_KP_DECIMAL: return ImGuiKey_KeypadDecimal;
        case GLFW_KEY_KP_DIVIDE: return ImGuiKey_KeypadDivide;
        case GLFW_KEY_KP_MULTIPLY: return ImGuiKey_KeypadMultiply;
        case GLFW_KEY_KP_SUBTRACT: return ImGuiKey_KeypadSubtract;
        case GLFW_KEY_KP_ADD: return ImGuiKey_KeypadAdd;
        case GLFW_KEY_KP_ENTER: return ImGuiKey_KeypadEnter;
        case GLFW_KEY_KP_EQUAL: return ImGuiKey_KeypadEqual;
        case GLFW_KEY_LEFT_SHIFT: return ImGuiKey_LeftShift;
        case GLFW_KEY_LEFT_CONTROL: return ImGuiKey_LeftCtrl;
        case GLFW_KEY_LEFT_ALT: return ImGuiKey_LeftAlt;
        case GLFW_KEY_LEFT_SUPER: return ImGuiKey_LeftSuper;
        case GLFW_KEY_RIGHT_SHIFT: return ImGuiKey_RightShift;
        case GLFW_KEY_RIGHT_CONTROL: return ImGuiKey_RightCtrl;
        case GLFW_KEY_RIGHT_ALT: return ImGuiKey_RightAlt;
        case GLFW_KEY_RIGHT_SUPER: return ImGuiKey_RightSuper;
        case GLFW_KEY_MENU: return ImGuiKey_Menu;
        case GLFW_KEY_0: return ImGuiKey_0;
        case GLFW_KEY_1: return ImGuiKey_1;
        case GLFW_KEY_2: return ImGuiKey_2;
        case GLFW_KEY_3: return ImGuiKey_3;
        case GLFW_KEY_4: return ImGuiKey_4;
        case GLFW_KEY_5: return ImGuiKey_5;
        case GLFW_KEY_6: return ImGuiKey_6;
        case GLFW_KEY_7: return ImGuiKey_7;
        case GLFW_KEY_8: return ImGuiKey_8;
        case GLFW_KEY_9: return ImGuiKey_9;
        case GLFW_KEY_A: return ImGuiKey_A;
        case GLFW_KEY_B: return ImGuiKey_B;
        case GLFW_KEY_C: return ImGuiKey_C;
        case GLFW_KEY_D: return ImGuiKey_D;
        case GLFW_KEY_E: return ImGuiKey_E;
        case GLFW_KEY_F: return ImGuiKey_F;
        case GLFW_KEY_G: return ImGuiKey_G;
        case GLFW_KEY_H: return ImGuiKey_H;
        case GLFW_KEY_I: return ImGuiKey_I;
        case GLFW_KEY_J: return ImGuiKey_J;
        case GLFW_KEY_K: return ImGuiKey_K;
        case GLFW_KEY_L: return ImGuiKey_L;
        case GLFW_KEY_M: return ImGuiKey_M;
        case GLFW_KEY_N: return ImGuiKey_N;
        case GLFW_KEY_O: return ImGuiKey_O;
        case GLFW_KEY_P: return ImGuiKey_P;
        case GLFW_KEY_Q: return ImGuiKey_Q;
        case GLFW_KEY_R: return ImGuiKey_R;
        case GLFW_KEY_S: return ImGuiKey_S;
        case GLFW_KEY_T: return ImGuiKey_T;
        case GLFW_KEY_U: return ImGuiKey_U;
        case GLFW_KEY_V: return ImGuiKey_V;
        case GLFW_KEY_W: return ImGuiKey_W;
        case GLFW_KEY_X: return ImGuiKey_X;
        case GLFW_KEY_Y: return ImGuiKey_Y;
        case GLFW_KEY_Z: return ImGuiKey_Z;
        case GLFW_KEY_F1: return ImGuiKey_F1;
        case GLFW_KEY_F2: return ImGuiKey_F2;
        case GLFW_KEY_F3: return ImGuiKey_F3;
        case GLFW_KEY_F4: return ImGuiKey_F4;
        case GLFW_KEY_F5: return ImGuiKey_F5;
        case GLFW_KEY_F6: return ImGuiKey_F6;
        case GLFW_KEY_F7: return ImGuiKey_F7;
        case GLFW_KEY_F8: return ImGuiKey_F8;
        case GLFW_KEY_F9: return ImGuiKey_F9;
        case GLFW_KEY_F10: return ImGuiKey_F10;
        case GLFW_KEY_F11: return ImGuiKey_F11;
        case GLFW_KEY_F12: return ImGuiKey_F12;
        default:
            return ImGuiKey_None;
    }
}

static std::unique_ptr<Texture> create_font() {
    ImFontAtlas* fonts = ImGui::GetIO().Fonts;
    fonts->AddFontDefault();

    ImFontConfig config;
    {
        config.MergeMode = true;
        config.PixelSnapH = true;
        config.OversampleV = 2;
        config.OversampleH = 2;
        config.FontDataOwnedByAtlas = false;
    }
    const ImWchar icon_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
    fonts->AddFontFromMemoryCompressedTTF(font_awesome_compressed_data, font_awesome_compressed_size, 13.0f, &config, icon_ranges);

    u8* font_data = nullptr;
    int width = 0;
    int height = 0;
    fonts->GetTexDataAsRGBA32(&font_data, &width, &height);

    const size_t bytes = 4 * width * height;

    TextureData data;
    data.format = ImageFormat::RGBA8_UNORM;
    data.size = glm::uvec2(width, height);
    data.data = std::make_unique<u8[]>(bytes);
    std::copy_n(font_data, bytes, data.data.get());

    return std::make_unique<Texture>(data);
}


static void char_callback(GLFWwindow*, unsigned characted) {
    ImGui::GetIO().AddInputCharacter(characted);
}


static void key_callback(GLFWwindow*, int key, int, int action, int mods) {
    auto& io = ImGui::GetIO();
    io.AddKeyEvent(ImGuiKey_ModCtrl, (mods & GLFW_MOD_CONTROL) != 0);
    io.AddKeyEvent(ImGuiKey_ModShift, (mods & GLFW_MOD_SHIFT) != 0);
    io.AddKeyEvent(ImGuiKey_ModAlt, (mods & GLFW_MOD_ALT) != 0);
    io.AddKeyEvent(ImGuiKey_ModSuper, (mods & GLFW_MOD_SUPER) != 0);
    io.AddKeyEvent(key_to_imgui(key), action == GLFW_PRESS);

}

static void mouse_pos_callback(GLFWwindow*, double xpos, double ypos) {
    ImGui::GetIO().AddMousePosEvent(float(xpos), float(ypos));
}

static void mouse_button_callback(GLFWwindow*, int button, int action, int) {
    ImGui::GetIO().AddMouseButtonEvent(button_to_imgui(button), action == GLFW_PRESS);
}

ImGuiRenderer::ImGuiRenderer(GLFWwindow* window) : _window(window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    _material.set_program(Program::from_files("imgui.frag", "imgui.vert"));
    _material.set_depth_test_mode(DepthTestMode::None);
    _material.set_blend_mode(BlendMode::Alpha);

    _font = create_font();

    glfwSetKeyCallback(_window, key_callback);
    glfwSetCharCallback(_window, char_callback);
    glfwSetCursorPosCallback(_window, mouse_pos_callback);
    glfwSetMouseButtonCallback(_window, mouse_button_callback);
}

void ImGuiRenderer::start() {
    auto& io = ImGui::GetIO();

    int w, h;
    glfwGetWindowSize(_window, &w, &h);
    io.DisplaySize = ImVec2(float(w), float(h));
    io.DeltaTime = update_delta_time();
    io.Fonts->TexID = _font.get();

    ImGui::NewFrame();
}

void ImGuiRenderer::finish() {
    ImGui::Render();
    render(ImGui::GetDrawData());
}


float ImGuiRenderer::update_delta_time() {
    const auto now = std::chrono::high_resolution_clock::now();
    const float dt = std::chrono::duration_cast<std::chrono::duration<float>>(now - _last).count();
    _last = now;
    return dt;
}

void ImGuiRenderer::render(const ImDrawData* draw_data) {
    if(!draw_data->TotalIdxCount || !draw_data->TotalVtxCount) {
        return;
    }

    const float width = (draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
    const float height = (draw_data->DisplaySize.y * draw_data->FramebufferScale.y);

    if(width <= 0.0f || height <= 0.0f) {
        return;
    }

    const ImVec2 clip_off = draw_data->DisplayPos;
    const ImVec2 clip_scale = draw_data->FramebufferScale;

    _material.set_uniform(HASH("viewport_size"), glm::vec2(draw_data->DisplaySize.x, draw_data->DisplaySize.y));
    _material.bind();

    glEnable(GL_SCISSOR_TEST);
    DEFER(glDisable(GL_SCISSOR_TEST));

    TypedBuffer<ImDrawIdx> index_buffer(nullptr, draw_data->TotalIdxCount);
    TypedBuffer<ImDrawVert> vertex_buffer(nullptr, draw_data->TotalVtxCount);

    {
        auto indices = index_buffer.map(AccessType::WriteOnly);
        auto vertices = vertex_buffer.map(AccessType::WriteOnly);

        size_t index_offset = 0;
        size_t vertex_offset = 0;
        for(int c = 0; c != draw_data->CmdListsCount; ++c) {
            const ImDrawList* cmd_list = draw_data->CmdLists[c];
            std::copy_n(cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size, &indices[index_offset]);
            std::copy_n(cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size, &vertices[vertex_offset]);
            vertex_offset += cmd_list->VtxBuffer.Size;
            index_offset += cmd_list->IdxBuffer.Size;
        }
    }

    index_buffer.bind(BufferUsage::Index);
    vertex_buffer.bind(BufferUsage::Attribute);

    byte* vertex_offset = nullptr;
    byte* index_offset = nullptr;
    for(int c = 0; c != draw_data->CmdListsCount; ++c) {
        const ImDrawList* cmd_list = draw_data->CmdLists[c];

        byte* drawn_index_offset = index_offset;
        for(int i = 0; i != cmd_list->CmdBuffer.Size; ++i) {
            const ImDrawCmd& cmd = cmd_list->CmdBuffer[i];

            ALWAYS_ASSERT(!cmd.UserCallback, "User callback not supported");

            const ImVec2 clip_min((cmd.ClipRect.x - clip_off.x) * clip_scale.x, (cmd.ClipRect.y - clip_off.y) * clip_scale.y);
            const ImVec2 clip_max((cmd.ClipRect.z - clip_off.x) * clip_scale.x, (cmd.ClipRect.w - clip_off.y) * clip_scale.y);
            if(!cmd.ElemCount || clip_max.x <= clip_min.x || clip_max.y <= clip_min.y) {
                continue;
            }

            glScissor(int(clip_min.x), int(height - clip_max.y), int(clip_max.x - clip_min.x), int(clip_max.y - clip_min.y));

            if(Texture* tex = static_cast<Texture*>(cmd.TextureId)) {
                tex->bind(0);
            }

            glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(ImDrawVert), vertex_offset);
            glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(ImDrawVert), vertex_offset + (2 * sizeof(float)));
            glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, false, sizeof(ImDrawVert), vertex_offset + (4 * sizeof(float)));
            glEnableVertexAttribArray(0);
            glEnableVertexAttribArray(1);
            glEnableVertexAttribArray(2);

            glDrawElements(GL_TRIANGLES, cmd.ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, reinterpret_cast<void*>(drawn_index_offset));
            drawn_index_offset += cmd.ElemCount * sizeof(ImDrawIdx);
        }

        vertex_offset += cmd_list->VtxBuffer.Size * sizeof(ImDrawVert);
        index_offset += cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx);
    }
}

}
