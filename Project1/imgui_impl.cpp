#if 0
#include <windows.h>
#include <stdint.h>
#include <gl/GL.h>
#include "imgui/imgui.h"

#include "math.h"
#include "memory.h"
#include "render.h"
#include "common.h"
#include "imgui_impl.h"
#endif

static HWND g_Window;
static bool         g_MouseJustPressed[3] = { false, false, false };
//static GLFWcursor*  g_MouseCursors[ImGuiMouseCursor_COUNT] = { 0 };

// OpenGL data
static GLuint       g_FontTexture = 0;

void ImGui_ImplGlfwGL2_RenderDrawData(ImDrawData* draw_data);

static POINT get_mouse_position_in_client()
{
    RECT window_rect;
    RECT client_rect;
    GetWindowRect(g_Window, &window_rect);
    GetClientRect(g_Window, &client_rect);

    POINT mouse_pos;
    GetCursorPos(&mouse_pos);

    int border_width = ((window_rect.right - window_rect.left) - client_rect.right) / 2;
    int top_border_height = ((window_rect.bottom - window_rect.top) - client_rect.bottom) - border_width;
    mouse_pos.x -= (window_rect.left + border_width);
    mouse_pos.y -= (window_rect.top + top_border_height);

    return mouse_pos;
}

void render_imgui_windows(SimState* state, bool* paused)
{
    static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    // 1. Show a simple window.
    // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets automatically appears in a window called "Debug".
    {
        ImGui::Text("Debug");
        POINT mouse_pos = get_mouse_position_in_client();
        ImGui::Text("mouse: %d, %d", mouse_pos.x, mouse_pos.y);
        Box last_box = state->render_state.last_imgui_window;
        ImGui::Text("content box: %f, %f, %f, %f", last_box.top_left.x, last_box.top_left.y, last_box.bottom_right.x, last_box.bottom_right.y);
        uint32_t selected_entity_index = get_selected_entity_index(state);
        ImGui::Text("Selected entity: %d/%d", selected_entity_index, state->num_entities);
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        ImGui::Separator();
        ImGui::Text("Simulation Control");
        ImGui::Text("dt = %f", state->time.effective_elapsed);
        ImGui::SliderFloat("Timescale", &state->time.timescale, 0, 1);
        if (ImGui::Button(*paused ? "Resume" : "Pause"))
        {
            *paused = !*paused;
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset"))
        {
            state->initialized = false;
        }
        if (ImGui::InputInt("Scope Range", &state->render_state.scope_range, 5, 20, 0))
        {
            CLAMP(5, state->render_state.scope_range, 360);
        }

        ImGui::Separator();
        ImGui::Text("Target Control");
        if (ImGui::Button("Add Target"))
        {
            if (state->render_state.control_event == ControlEvent_None)
            {
                state->render_state.control_event = ControlEvent_SetTargetPosition;
                state->render_state.control_event_entity_index = add_entity(state, EntityKind_Aircraft);
            }
        }

        if (selected_entity_index > 0)
        {
            EntityType* selected_entity = state->entities + selected_entity_index;

            if (selected_entity_index != state->ownship_index)
            {
                ImGui::SameLine();
                if (ImGui::Button("Remove Target"))
                {
                    remove_entity(state, selected_entity_index);
                }
            }

            ImGui::Separator();
            ImGui::Text("Selected Target Data");
            if (selected_entity)
            {
                ImGui::InputFloat3("Lat, Lon, Alt", (float*)&selected_entity->geo_pos, ImGuiInputTextFlags_ReadOnly);
                ImGui::InputFloat3("ECEF X, Y, Z", (float*)&selected_entity->ecef_pos, ImGuiInputTextFlags_ReadOnly);
                ImGui::InputFloat3("N, E, D", (float*)&selected_entity->ned_pos, ImGuiInputTextFlags_ReadOnly);
                ImGui::InputFloat("Heading", &selected_entity->aircraft.heading);
            }

            ImGui::Separator();
            ImGui::Text("Shoot List");
            if (target_in_shoot_list(state, selected_entity_index))
            {
                if (ImGui::Button("Remove from shoot list"))
                {
                    remove_from_shoot_list(state, selected_entity);
                }
            }
            else
            {
                if (ImGui::Button("Add to shoot list"))
                {
                    add_to_shoot_list(state, selected_entity);
                }
            }
        }
        
        ImVec2 min = ImGui::GetWindowPos();
        ImVec2 max = ImGui::GetWindowSize();
        Vec2 top_left = Vec2{ min.x, min.y };
        Vec2 bottom_right = top_left + Vec2{ max.x, max.y };
        state->render_state.last_imgui_window = Box{ top_left, bottom_right };
    }

    ImGui::Render();
    ImGui_ImplGlfwGL2_RenderDrawData(ImGui::GetDrawData());
}

// OpenGL2 Render function.
// (this used to be set in io.RenderDrawListsFn and called by ImGui::Render(), but you can now call this directly from your main loop)
// Note that this implementation is little overcomplicated because we are saving/setting up/restoring every OpenGL state explicitly, in order to be able to run within any OpenGL engine that doesn't do so. 
void ImGui_ImplGlfwGL2_RenderDrawData(ImDrawData* draw_data)
{
    // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
    ImGuiIO& io = ImGui::GetIO();
    int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
    int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
    if (fb_width == 0 || fb_height == 0)
        return;
    draw_data->ScaleClipRects(io.DisplayFramebufferScale);

    // We are using the OpenGL fixed pipeline to make the example code simpler to read!
    // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, vertex/texcoord/color pointers, polygon fill.
    GLint last_texture; glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    GLint last_polygon_mode[2]; glGetIntegerv(GL_POLYGON_MODE, last_polygon_mode);
    GLint last_viewport[4]; glGetIntegerv(GL_VIEWPORT, last_viewport);
    GLint last_scissor_box[4]; glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_TRANSFORM_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnable(GL_TEXTURE_2D);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    //glUseProgram(0); // You may want this if using this code in an OpenGL 3+ context where shaders may be bound

    // Setup viewport, orthographic projection matrix
    glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0f, io.DisplaySize.x, io.DisplaySize.y, 0.0f, -1.0f, +1.0f);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Render command lists
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        const ImDrawVert* vtx_buffer = cmd_list->VtxBuffer.Data;
        const ImDrawIdx* idx_buffer = cmd_list->IdxBuffer.Data;
        glVertexPointer(2, GL_FLOAT, sizeof(ImDrawVert), (const GLvoid*)((const char*)vtx_buffer + IM_OFFSETOF(ImDrawVert, pos)));
        glTexCoordPointer(2, GL_FLOAT, sizeof(ImDrawVert), (const GLvoid*)((const char*)vtx_buffer + IM_OFFSETOF(ImDrawVert, uv)));
        glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(ImDrawVert), (const GLvoid*)((const char*)vtx_buffer + IM_OFFSETOF(ImDrawVert, col)));

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback)
            {
                pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
                glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
                glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer);
            }
            idx_buffer += pcmd->ElemCount;
        }
    }

    // Restore modified state
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindTexture(GL_TEXTURE_2D, (GLuint)last_texture);
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();
    glPolygonMode(GL_FRONT, (GLenum)last_polygon_mode[0]); glPolygonMode(GL_BACK, (GLenum)last_polygon_mode[1]);
    glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
    glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
}

#if 0
static const char* ImGui_ImplGlfwGL2_GetClipboardText(void* user_data)
{
    return glfwGetClipboardString((GLFWwindow*)user_data);
}

static void ImGui_ImplGlfwGL2_SetClipboardText(void* user_data, const char* text)
{
    glfwSetClipboardString((GLFWwindow*)user_data, text);
}
#endif


void ImGui_MouseButtonCallback(int button, bool pressed)
{
    if (pressed && button >= 0 && button < 3)
        g_MouseJustPressed[button] = true;
}


#if 0
void ImGui_ImplGlfw_ScrollCallback(GLFWwindow*, double xoffset, double yoffset)
{
    ImGuiIO& io = ImGui::GetIO();
    io.MouseWheelH += (float)xoffset;
    io.MouseWheel += (float)yoffset;
}
#endif

void ImGui_ImplGlfw_KeyCallback(uint32_t key, UiKeyState action)
{
    ImGuiIO& io = ImGui::GetIO();
    if (action == UiKeyPressed)
        io.KeysDown[key] = true;
    if (action == UiKeyReleased)
        io.KeysDown[key] = false;

    io.KeyCtrl = io.KeysDown[VK_LCONTROL] || io.KeysDown[VK_RCONTROL];
    io.KeyShift = io.KeysDown[VK_LSHIFT] || io.KeysDown[VK_RSHIFT];
    io.KeyAlt = io.KeysDown[VK_LMENU] || io.KeysDown[VK_RMENU];
    io.KeySuper = io.KeysDown[VK_LWIN] || io.KeysDown[VK_RWIN];
}

void ImGui_ImplGlfw_CharCallback(unsigned int c)
{
    ImGuiIO& io = ImGui::GetIO();
    if (c > 0 && c < 0x10000)
        io.AddInputCharacter((unsigned short)c);
}

bool ImGui_ImplGlfwGL2_CreateDeviceObjects()
{
    // Build texture atlas
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);  
    
    GLint last_texture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    glGenTextures(1, &g_FontTexture);
    glBindTexture(GL_TEXTURE_2D, g_FontTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    // Store our identifier
    io.Fonts->TexID = (void *)(intptr_t)g_FontTexture;

    // Restore state
    glBindTexture(GL_TEXTURE_2D, last_texture);

    return true;
}

void ImGui_ImplGlfwGL2_InvalidateDeviceObjects()
{
    if (g_FontTexture)
    {
        glDeleteTextures(1, &g_FontTexture);
        ImGui::GetIO().Fonts->TexID = 0;
        g_FontTexture = 0;
    }
}

bool ImGui_ImplGlfwGL2_Init(HWND window)
{
    // Setup ImGui binding
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    g_Window = window;
    io.ImeWindowHandle = g_Window;

    io.KeyMap[ImGuiKey_Tab] = VK_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
    io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
    io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
    io.KeyMap[ImGuiKey_Home] = VK_HOME;
    io.KeyMap[ImGuiKey_End] = VK_END;
    io.KeyMap[ImGuiKey_Insert] = VK_INSERT;
    io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
    io.KeyMap[ImGuiKey_Space] = VK_SPACE;
    io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
    io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;

#if 0
    io.SetClipboardTextFn = ImGui_ImplGlfwGL2_SetClipboardText;
    io.GetClipboardTextFn = ImGui_ImplGlfwGL2_GetClipboardText;
    io.ClipboardUserData = g_Window;

    // Load cursors
    // FIXME: GLFW doesn't expose suitable cursors for ResizeAll, ResizeNESW, ResizeNWSE. We revert to arrow cursor for those.
    g_MouseCursors[ImGuiMouseCursor_Arrow] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    g_MouseCursors[ImGuiMouseCursor_TextInput] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
    g_MouseCursors[ImGuiMouseCursor_ResizeAll] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    g_MouseCursors[ImGuiMouseCursor_ResizeNS] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
    g_MouseCursors[ImGuiMouseCursor_ResizeEW] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
    g_MouseCursors[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    g_MouseCursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);

    if (install_callbacks)
        ImGui_ImplGlfw_InstallCallbacks(window);
#endif
    ImGui::StyleColorsDark();

    return true;
}

void ImGui_ImplGlfwGL2_Shutdown()
{
#if 0
    // Destroy GLFW mouse cursors
    for (ImGuiMouseCursor cursor_n = 0; cursor_n < ImGuiMouseCursor_COUNT; cursor_n++)
        glfwDestroyCursor(g_MouseCursors[cursor_n]);
    memset(g_MouseCursors, 0, sizeof(g_MouseCursors));
#endif

    // Destroy OpenGL objects
    ImGui_ImplGlfwGL2_InvalidateDeviceObjects();
}

void ImGui_ImplGlfwGL2_NewFrame(float dt)
{
    if (!g_FontTexture)
        ImGui_ImplGlfwGL2_CreateDeviceObjects();

    ImGuiIO& io = ImGui::GetIO();

    // Setup display size (every frame to accommodate for window resizing)
    int w, h;
    RECT client_rect;
    GetClientRect(g_Window, &client_rect);
    w = client_rect.right - client_rect.left;
    h = client_rect.bottom - client_rect.top;
    io.DisplaySize = ImVec2((float)w, (float)h);
    io.DisplayFramebufferScale = ImVec2(1, 1);

    io.DeltaTime = dt;

    // Setup inputs
    // (we already got mouse wheel, keyboard keys & characters from glfw callbacks polled in glfwPollEvents())
    if (g_Window == GetFocus())
    {
        if (io.WantMoveMouse)
        {
            SetCursorPos((int)io.MousePos.x, (int)io.MousePos.y);   // Set mouse position if requested by io.WantMoveMouse flag (used when io.NavMovesTrue is enabled by user and using directional navigation)
        }
        else
        {
            POINT mouse_pos = get_mouse_position_in_client();
            io.MousePos = ImVec2((float)mouse_pos.x, (float)mouse_pos.y);
        }
    }
    else
    {
        io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
    }

    
    io.MouseDown[0] = g_MouseJustPressed[0] || GetAsyncKeyState(VK_LBUTTON) >> 15;
    g_MouseJustPressed[0] = false;
    io.MouseDown[1] = g_MouseJustPressed[1] || GetAsyncKeyState(VK_MBUTTON) >> 15;
    g_MouseJustPressed[1] = false;
    io.MouseDown[2] = g_MouseJustPressed[2] || GetAsyncKeyState(VK_RBUTTON) >> 15;
    g_MouseJustPressed[2] = false;
    
#if 0
    // Update OS/hardware mouse cursor if imgui isn't drawing a software cursor
    ImGuiMouseCursor cursor = ImGui::GetMouseCursor();
    if (io.MouseDrawCursor || cursor == ImGuiMouseCursor_None)
    {
        glfwSetInputMode(g_Window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    }
    else
    {
        glfwSetCursor(g_Window, g_MouseCursors[cursor] ? g_MouseCursors[cursor] : g_MouseCursors[ImGuiMouseCursor_Arrow]);
        glfwSetInputMode(g_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
#endif

    // Start the frame. This call will update the io.WantCaptureMouse, io.WantCaptureKeyboard flag that you can use to dispatch inputs (or not) to your application.
    ImGui::NewFrame();
}
