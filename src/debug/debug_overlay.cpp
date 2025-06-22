#include "debug/debug_overlay.h"

#include "Common.h"
#include "Matrix.h"
#include "Version.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include <imgui_impl_sdlrenderer3.h>

namespace DebugOverlay
{
void Init(const AppContext *context)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io{ImGui::GetIO()};

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL3_InitForSDLRenderer(context->MainWindow(), context->MainRenderer());
    ImGui_ImplSDLRenderer3_Init(context->MainRenderer());
}

void Update(AppContext *context, const SDL_Event *event)
{
    ImGui_ImplSDL3_ProcessEvent(event);

    if (show_led_matrix)
    {
        if (!context->LedMatrixRenderer())
        {
            const auto win = CreateWindow("LED Matrix", width * 50, height * 50);
            SDL_SetWindowFocusable(win->window(), false);
            SDL_SetRenderVSync(win->renderer(), SDL_RENDERER_VSYNC_ADAPTIVE);
            SDL_SetWindowPosition(win->window(), 0, 0);
            SDL_ShowWindow(win->window());

            const auto windowId = SDL_GetWindowID(win->window());
            context->SetMatrix(new Matrix(windowId, win->renderer(), width, height));
        }
    }
    else
    {
        if (context->LedMatrixRenderer())
        {
            int window_count = 0;
            if (SDL_Window **windows = SDL_GetWindows(&window_count))
            {
                for (int i = 0; i < window_count; ++i)
                {
                    if (SDL_Window *window = windows[i]; context->LedMatrixId() == SDL_GetWindowID(window))
                    {
                        SDL_DestroyRenderer(context->LedMatrixRenderer());
                        SDL_DestroyWindow(window);
                        break;
                    }
                }
                SDL_free(windows);
            }

            SDL_DestroyRenderer(context->LedMatrixRenderer());

            context->SetMatrix(nullptr);
        }
    }
}

void Render(const AppContext *context)
{
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    if (show_debug_window && ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Config"))
        {
            ImGui::Checkbox("Show LED Matrix", &show_led_matrix);
            ImGui::Checkbox("Show Unhandled Events", &show_unhandled_events);

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help"))
        {
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
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), context->MainRenderer());
}

void Cleanup()
{
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}
} // namespace DebugOverlay