#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <inttypes.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <gl/GL.h>
#include <gl/GLU.h>


#if !defined(COMPILER_MSVC)
#define COMPILER_MSVC 0
#endif

#if !defined(COMPILER_LLVM)
#define COMPILER_LLVM 0
#endif

#if !COMPILER_MSVC && !COMPILER_LLVM
#if _MSC_VER
#undef COMPILER_MSVC
#define COMPILER_MSVC 1
#else
#undef COMPILER_LLVM
#define COMPILER_LLVM 1
#endif
#endif

#if COMPILER_MSVC
#include <intrin.h>
#endif

#define STB_TRUETYPE_IMPLEMENTATION  // force following include to generate implementation
#include "stb_truetype.h"

#include "math.h"
#include "math.cpp"
#include "memory.h"
#include "memory.cpp"
#include "render.h"
#include "common.h"
#include "imgui_impl.h"
#include "render.cpp"
#include "common.cpp"
#include "simulation.cpp"

static bool global_running;
static bool global_paused;
static RenderState* global_render_state;

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
            render(global_render_state, dimensions);
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
        SimState state = { 0 };
        InitializeMemoryArena(&state.sim_arena, MEGABYTES(200));
        InitializeMemoryArena(&state.render_state.arena, MEGABYTES(100));
        global_render_state = &state.render_state;
        
        HWND window = InitOpenGL(hInstance);
        init_render_state(global_render_state);

        if (window)
        {
            global_running = true;
            global_paused = false;
            
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


                QueryPerformanceCounter(&end_counter);
                float elapsed_seconds = (float)(end_counter.QuadPart - start_counter.QuadPart) / (float)counter_frequency;
                start_counter = end_counter;
                state.dt = elapsed_seconds;

                if (!global_paused || !state.initialized)
                {
                    UpdateSimulation(&state);
                }
                add_static_render_objects(&state, global_render_state);

                HDC device_context = GetDC(window);
                WindowDimension dimensions = GetWindowDimension(window);
                add_dynamic_render_objects(&state, dimensions);
                render(global_render_state, dimensions);

                ImGui_ImplGlfwGL2_NewFrame(state.dt);
                render_imgui_windows(&state, &global_paused);

                SwapBuffers(device_context);
                ReleaseDC(window, device_context);
            }
        }
    }
    

}