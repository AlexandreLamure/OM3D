#include "ImGuiRenderer.h"

#include <TypedBuffer.h>

#include <glm/vec2.hpp>

#include <imgui/imgui.h>
#include <glad/glad.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <algorithm>

static std::unique_ptr<Texture> create_font() {
    ImFontAtlas* fonts = ImGui::GetIO().Fonts;

    fonts->AddFontDefault();
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

ImGuiRenderer::ImGuiRenderer() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    _material._program = std::make_shared<Program>(Program::from_files("imgui.frag", "imgui.vert"));
    _material._depth_test_mode = DepthTestMode::None;
    _material._blend_mode = BlendMode::Alpha;

    _font = create_font();
}

void ImGuiRenderer::render(GLFWwindow* window) {
    auto& io = ImGui::GetIO();

    int w, h;
    glfwGetWindowSize(window, &w, &h);
    io.DisplaySize = ImVec2(float(w), float(h));
    io.DeltaTime = update_dt();
    io.Fonts->TexID = _font.get();

    ImGui::NewFrame();
    ImGui::ShowDemoWindow();
    ImGui::Render();

    render(ImGui::GetDrawData());
}


float ImGuiRenderer::update_dt() {
    const auto now = std::chrono::high_resolution_clock::now();
    const float dt = std::chrono::duration_cast<std::chrono::duration<float>>(now - _last).count();
    _last = now;
    return dt;
}

void ImGuiRenderer::render(const ImDrawData* draw_data) {
    const float width = (draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
    const float height = (draw_data->DisplaySize.y * draw_data->FramebufferScale.y);

    if(width <= 0.0f || height <= 0.0f) {
        return;
    }

    const ImVec2 clip_off = draw_data->DisplayPos;
    const ImVec2 clip_scale = draw_data->FramebufferScale;

    _material._program->set_uniform(HASH("viewport_size"), glm::vec2(draw_data->DisplaySize.x, draw_data->DisplaySize.y));
    _material.bind();

    glEnable(GL_SCISSOR_TEST);
    DEFER(glDisable(GL_SCISSOR_TEST));

    TypedBuffer<ImDrawIdx> index_buffer(nullptr, draw_data->TotalIdxCount);
    TypedBuffer<ImDrawVert> vertex_buffer(nullptr, draw_data->TotalVtxCount);

    {
        auto indices = index_buffer.map(MappingType::WriteOnly);
        auto vertices = vertex_buffer.map(MappingType::WriteOnly);

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

    u8* vertex_offset = nullptr;
    u8* index_offset = nullptr;
    for(int c = 0; c != draw_data->CmdListsCount; ++c) {
        const ImDrawList* cmd_list = draw_data->CmdLists[c];

        u8* drawn_index_offset = index_offset;
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
