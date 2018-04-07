#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <inttypes.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <gl/GL.h>

#include "common.c"

typedef struct WindowDimension {
    int width;
    int height;
} WindowDimension;

typedef struct RenderState {
    Bitmap target_bitmap;
    GLuint target_texture_id;
} RenderState;

bool global_running;
bool global_paused;
RenderState global_render_state;

Vec3 ColorRed = { 1.0f, 0.0f, 0.0f };

static void InitRenderState()
{
    RenderState* state = &global_render_state;
    state->target_bitmap = Win32LoadBitmap("./res/target.bmp");
        
    glGenTextures(1, &state->target_texture_id);
    glBindTexture(GL_TEXTURE_2D, state->target_texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, state->target_bitmap.width, state->target_bitmap.height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, state->target_bitmap.pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glEnable(GL_TEXTURE_2D);
}

static WindowDimension GetWindowDimension(HWND window)
{
    WindowDimension result;

    RECT client_rect;
    GetClientRect(window, &client_rect);
    result.width = client_rect.right - client_rect.left;
    result.height = client_rect.bottom - client_rect.top;

    return result;
}

static void DrawCircle(int center_x, int center_y, float radius, Vec3 color)
{
    glBegin(GL_LINE_LOOP);
    glColor3f(color.r, color.g, color.b);
    int num_points = 80;
    float scalar = TWO_PI / num_points;
    for (int i = 0; i < num_points; i++)
    {
        float x = center_x + radius * cosf(i * scalar);
        float y = center_y + radius * sinf(i * scalar);
        glVertex2f(x, y);
    }
    glEnd();
}

static void Win32DisplayBufferInWindow(HDC device_context, WindowDimension dimensions)
{
    glViewport(0, 0, dimensions.width, dimensions.height);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    float a = 2.0f / dimensions.width;
    float b = 2.0f / dimensions.height;
    float proj[] = {
         a,  0,  0,  0,
         0,  b,  0,  0,
         0,  0,  1,  0,
        -1, -1,  0,  1
    };
    glLoadMatrixf(proj);

    glBegin(GL_TRIANGLES);

    Vec2 center = { dimensions.width * 0.5f, dimensions.height * 0.5f };
    Vec2 min_p = { center.x - 100, center.y - 100};
    Vec2 max_p = { center.x + 100, center.y + 100 };

    glColor3f(1.0f, 1.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(min_p.x, min_p.y);

    glTexCoord2f(1.0f, 0.0f);
    //glColor3f(0.0f, 1.0f, 0.0f);
    glVertex2f(max_p.x, min_p.y);

    glTexCoord2f(1.0f, 1.0f);
    //glColor3f(0.0f, 0.0f, 1.0f);
    glVertex2f(max_p.x, max_p.y);

    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(min_p.x, min_p.y);

    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(max_p.x, max_p.y);

    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(min_p.x, max_p.y);

    glEnd();

    float radius = 0.45f * (float)dimensions.height;
    DrawCircle(dimensions.width / 2, dimensions.height / 2, radius, ColorRed);

    SwapBuffers(device_context);
}

static void Win32InitOpenGL(HWND window)
{
    HDC device_context = GetDC(window);

    PIXELFORMATDESCRIPTOR pixel_format = { 0 };
    pixel_format.nSize = sizeof(pixel_format);
    pixel_format.nVersion = 1;
    pixel_format.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
    pixel_format.cColorBits = 32;
    pixel_format.cAlphaBits = 8;

    int pixel_format_index = ChoosePixelFormat(device_context, &pixel_format);
    PIXELFORMATDESCRIPTOR suggested_pixel_format;
    DescribePixelFormat(device_context, pixel_format_index, sizeof(PIXELFORMATDESCRIPTOR), &suggested_pixel_format);
    SetPixelFormat(device_context, pixel_format_index, &suggested_pixel_format);

    HGLRC opengl_rc = wglCreateContext(device_context);
    if (!wglMakeCurrent(device_context, opengl_rc))    
    {
        assert(!"Failed to create OpenGL context!");
    }
    ReleaseDC(window, device_context);
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
            Win32DisplayBufferInWindow(device_context, dimensions);
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
                // TODO(scott): implement this
                uint32_t vk_code = (uint32_t)message.wParam;
                bool was_down = ((message.lParam & (1 << 30)) != 0);
                bool is_doan = ((message.lParam & (1 << 31)) == 0);
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

            Win32InitOpenGL(window);

            InitRenderState();

            while (global_running)
            {
                Win32ProcessPendingMessages();

                if (!global_paused)
                {
                    // update
                }

                HDC device_context = GetDC(window);
                WindowDimension dimensions = GetWindowDimension(window);
                Win32DisplayBufferInWindow(device_context, dimensions);
                ReleaseDC(window, device_context);
            }
        }
    }
    

}