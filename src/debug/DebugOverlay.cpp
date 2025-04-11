#include "debug/DebugOverlay.h"

#include <imgui_impl_sdlrenderer3.h>
#include "Common.h"
#include "Version.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "ui/Matrix.h"

namespace DebugOverlay {
void init(const AppContext* context) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io{ImGui::GetIO()};

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL3_InitForSDLRenderer(context->window(), context->renderer());
    ImGui_ImplSDLRenderer3_Init(context->renderer());
}

void update(AppContext* context, const SDL_Event* event) {
    ImGui_ImplSDL3_ProcessEvent(event);

    if(show_led_matrix) {
        if(context->matrix_window() == nullptr) {
            const auto win = createWindow("LED Matrix", 32 * 50, 8 * 50);
            SDL_SetWindowFocusable(win->window(), false);
            SDL_SetRenderVSync(win->renderer(), SDL_RENDERER_VSYNC_ADAPTIVE);
            SDL_SetWindowPosition(win->window(), 0, 0);
            SDL_ShowWindow(win->window());

            context->setMatrix(new Matrix(win));
        }
    } else {
        if(context->matrix_window() != nullptr) {
            SDL_DestroyWindow(context->matrix_window());

            context->setMatrix(nullptr);
        }
    }
}

void render(const AppContext* context) {
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    if(show_debug_window && ImGui::BeginMainMenuBar()) {
        if(ImGui::BeginMenu("Config")) {
            ImGui::Checkbox("Show LED Matrix", &show_led_matrix);
            ImGui::Checkbox("Show Unhandled Events", &show_unhandled_events);

            ImGui::EndMenu();
        }
        if(ImGui::BeginMenu("Help")) {
            ImGui::Text("FPS: %.2f", ImGui::GetIO().Framerate);
            ImGui::SeparatorText("App Info");
            ImGui::Text("Project: %s", MyProject.c_str());
            ImGui::Text("Version: %s", MyProjectVersion.c_str());
            ImGui::Text("Build Date: %s", MyProjectBuildDate.c_str());
            ImGui::Text("ImGui Version: %s", ImGui::GetVersion());

            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    // Rendering
    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), context->renderer());
}

void cleanup() {
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}
} // namespace DebugOverlay
