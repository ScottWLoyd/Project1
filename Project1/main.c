#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <inttypes.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <gl/GL.h>

typedef int bool;
#define true (1==1)
#define false (1==0)

#include "imgui_impl.h"

#define MAX(x,y) (x > y ? x : y)

#include "math.c"
#include "memory.c"
#include "render.c"
#include "common.c"
#include "simulation.c"

static bool global_running;
static bool global_paused;
static RenderState* global_render_state;
static UiState global_ui_state;

static WindowDimension GetWindowDimension(HWND window)
{
    WindowDimension result;

    RECT client_rect;
    GetClientRect(window, &client_rect);
    result.width = client_rect.right - client_rect.left;
    result.height = client_rect.bottom - client_rect.top;

    return result;
}

static LRESULT WindowProc(HWND window, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    switch (uMsg)
    {
        case WM_CLOSE:
        case WM_DESTROY: {
            global_running = false;
        } break;

        case WM_ACTIVATEAPP: {
            
        } break;

        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP: {
            assert(!"Keyboard input came in through a non-dispatch message!");
        } break;

        case WM_PAINT: {
            PAINTSTRUCT paint;
            HDC device_context = BeginPaint(window, &paint);
            WindowDimension dimensions = GetWindowDimension(window);
            Render(global_render_state, dimensions);
            SwapBuffers(device_context);
            EndPaint(window, &paint);
        } break;

        default:
            result = DefWindowProc(window, uMsg, wParam, lParam);
    }
    return result;
}

static void Win32ProcessPendingMessages()
{
    MSG message;
    while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
    {
        switch (message.message)
        {
            case WM_QUIT: {
                global_running = false;
            } break;

            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP: {                
                uint32_t vk_code = (uint32_t)message.wParam;
                bool was_down = ((message.lParam & (1 << 30)) != 0);
                bool is_down = ((message.lParam & (1 << 31)) == 0);

                char c = (char)(MapVirtualKey(vk_code, MAPVK_VK_TO_CHAR) & 0xff);
                if (isprint(c) && is_down)
                {
                    ImGui_ImplGlfw_CharCallback(c);
                }
                else
                {
                    ImGui_ImplGlfw_KeyCallback(vk_code, is_down ? UiKeyPressed : UiKeyReleased);
                }
            } break;

            case WM_LBUTTONDOWN: {
                ImGui_MouseButtonCallback(0, true);
            } break;
            case WM_LBUTTONUP: {
                ImGui_MouseButtonCallback(0, false);
            } break;
            case WM_MBUTTONDOWN: {
                ImGui_MouseButtonCallback(1, true);
            } break;
            case WM_MBUTTONUP: {
                ImGui_MouseButtonCallback(1, false);
            } break;
            case WM_RBUTTONDOWN: {
                ImGui_MouseButtonCallback(2, true);
            } break;
            case WM_RBUTTONUP: {
                ImGui_MouseButtonCallback(2, false);
            } break;


            default: {
                TranslateMessage(&message);
                DispatchMessageA(&message);
            } break;
        }
    }
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEX window_class = {0};
    window_class.cbSize = sizeof(window_class);
    window_class.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
    window_class.lpfnWndProc = (WNDPROC)WindowProc;
    window_class.hInstance = hInstance;
    window_class.hCursor = LoadCursor(0, IDC_ARROW);
    window_class.lpszClassName = "Project1 Window Class";
    
    if (RegisterClassEx(&window_class))
    {
        int width = 600;
        int height = 800;
        HWND window = CreateWindow("Project1 Window Class", "Project1 Window",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE, 
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
            0, 0, hInstance, 0);
        
        if (window)
        {
            global_running = true;
            global_paused = false;
            
            InitOpenGL(window);

            SimState state = { 0 };
            InitializeMemoryArena(&state.sim_arena, MEGABYTES(200));
            InitializeMemoryArena(&state.render_state.arena, MEGABYTES(100));
            global_render_state = &state.render_state;

            ImGui_ImplGlfwGL2_Init(window);

            LARGE_INTEGER start_counter, end_counter;
            QueryPerformanceCounter(&start_counter);
            LARGE_INTEGER frequency_result;
            int64_t counter_frequency;
            QueryPerformanceFrequency(&frequency_result);
            counter_frequency = frequency_result.QuadPart;

            while (global_running)
            {
                Win32ProcessPendingMessages();

                AddStaticRenderObjects(global_render_state);

                QueryPerformanceCounter(&end_counter);
                float elapsed_seconds = (float)(end_counter.QuadPart - start_counter.QuadPart) / (float)counter_frequency;
                start_counter = end_counter;
                state.dt = global_ui_state.dt = elapsed_seconds;

                if (!global_paused)
                {
                    UpdateSimulation(&state);
                }

                HDC device_context = GetDC(window);
                WindowDimension dimensions = GetWindowDimension(window);
                Render(global_render_state, dimensions);
                
                ImGui_ImplGlfwGL2_NewFrame(state.dt);
                render_imgui_windows(&global_ui_state);

                SwapBuffers(device_context);
                ReleaseDC(window, device_context);
                
            }
        }
    }
    

}