/*
Roadmap:

    - Kinematic simulation
        - velocity/acceleration integration
    - 1553 packing/unpacking
    - Ethernet packing/unpacking
    - Scenario input file reading
    - Configuration file:
        - Simulated vs HITL subsystems
        - pointer to scenario?
        - pointer to hotas config
    - Hotloading for scenarios/config
    - Armament config
        - AIM-9X threshold
        - AIM-120 objective
    - Radar sim
        - scan pattern & volume
        - modes SWT/STT
    - JHMCS sim
        - graphical control of LOS
    - Display az & el LOS data
    - Visually display LOS locklines
        - to include elevation somehow
    - HOTAS input
        - maybe a list of mappings loaded from a configuration file?
    - PAD input    

Done but needs testing:

    - Better window layout
        - align SIT to left/right, maximize remaining space for data dump/control, etc.

*/

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
#include "render_group.h"
#include "render.h"
#include "common.h"
#include "imgui_impl.h"
#include "render_group.cpp"
#include "render.cpp"
#include "nav.cpp"
#include "common.cpp"
#include "simulation.cpp"
#include "imgui/imgui.h"
#include "imgui_impl.cpp"

static bool global_running;
static bool global_paused;
static RenderState* global_render_state;

static WindowDimension get_window_dimension(HWND window)
{
    WindowDimension result;

    RECT client_rect;
    GetClientRect(window, &client_rect);
    result.width = client_rect.right - client_rect.left;
    result.height = client_rect.bottom - client_rect.top;

    return result;
}

static LRESULT window_proc(HWND window, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
            render(global_render_state);
            SwapBuffers(device_context);
            EndPaint(window, &paint);
        } break;

        default:
            result = DefWindowProc(window, uMsg, wParam, lParam);
    }
    return result;
}

static void dispatch_mouse_button_event(RenderState* state, uint32_t button, bool pressed)
{
    if (box_contains_point(state->last_imgui_window,
        convert_point_to_window_space(state, state->mouse_pos)))
    {
        ImGui_MouseButtonCallback(0, pressed);
    }
    else
    {
        state->mouse_buttons[0] = pressed;
    }
}

static void win32_process_pending_messages()
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

            case WM_MOUSEMOVE: {
                Vec2 mouse_pos = Vec2{ (float)(message.lParam & 0xffff), 
                                       (float)((message.lParam >> 16) & 0xffff) };
                
                global_render_state->mouse_pos = convert_point_to_render_space(global_render_state, mouse_pos);
            } break;

            case WM_LBUTTONDOWN: {
                dispatch_mouse_button_event(global_render_state, 0, true);
            } break;
            case WM_LBUTTONUP: {
                dispatch_mouse_button_event(global_render_state, 0, false);
            } break;
            case WM_MBUTTONDOWN: {
                dispatch_mouse_button_event(global_render_state, 1, true);
            } break;
            case WM_MBUTTONUP: {
                dispatch_mouse_button_event(global_render_state, 1, false);
            } break;
            case WM_RBUTTONDOWN: {
                dispatch_mouse_button_event(global_render_state, 2, true);
            } break;
            case WM_RBUTTONUP: {
                dispatch_mouse_button_event(global_render_state, 2, false);
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
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;

    WNDCLASSEX window_class = {0};
    window_class.cbSize = sizeof(window_class);
    window_class.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
    window_class.lpfnWndProc = (WNDPROC)window_proc;
    window_class.hInstance = hInstance;
    window_class.hCursor = LoadCursor(0, IDC_ARROW);
    window_class.lpszClassName = "Project1 Window Class";
    
    if (RegisterClassEx(&window_class))
    {   
        MemoryArena sim_arena;
        InitializeMemoryArena(&sim_arena, MEGABYTES(200));
        SimState* state = push_struct(&sim_arena, SimState);
        zero_struct(*state);
        state->sim_arena = &sim_arena;
        InitializeMemoryArena(&state->render_state.arena, MEGABYTES(100));
        state->time.timescale = 1.0f;        
        global_render_state = &state->render_state;
        
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
                win32_process_pending_messages();

                perform_ui_processing(state);

                // NOTE(scott): wait to clear render state until here so that we can 
                // use it in the event handling above.
                global_render_state->arena.next = global_render_state->arena.base;
                global_render_state->num_render_groups = 0;

                QueryPerformanceCounter(&end_counter);
                float elapsed_seconds = (float)(end_counter.QuadPart - start_counter.QuadPart) / (float)counter_frequency;
                start_counter = end_counter;
                state->time.dt = elapsed_seconds;
                state->time.effective_elapsed = state->time.dt * state->time.timescale;

                if (!global_paused || !state->initialized)
                {
                    update_simulation(state);
                }

                state->render_state.window_dimensions = get_window_dimension(window);
                Vec2 center = get_sit_center_point(&state->render_state);
                float min_dimension = (float)MIN(center.x, center.y);
                global_render_state->feet_to_pixels = min_dimension / NM_TO_FT(global_render_state->scope_range);

                add_static_render_objects(state);
                add_dynamic_render_objects(state);
                render(global_render_state);

                ImGui_ImplGlfwGL2_NewFrame(state->time.dt);
                render_imgui_windows(state, &global_paused);

                HDC device_context = GetDC(window);
                SwapBuffers(device_context);
                ReleaseDC(window, device_context);
            }
        }
    }
    

}