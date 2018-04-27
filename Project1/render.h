const Vec3 ColorRed = { 1.0f, 0.0f, 0.0f };
const Vec3 ColorGreen = { 0.0f, 1.0f, 0.0f };
const Vec3 ColorBlue = { 0.0f, 0.0f, 1.0f };
const Vec3 ColorMagenta = { 1.0f, 0.0f, 1.0f };
const Vec3 ColorYellow = { 1.0f, 1.0f, 0.0f };
const Vec3 ColorCyan = { 0.0f, 1.0f, 1.0f };
const Vec3 ColorWhite = { 1.0f, 1.0f, 1.0f };
const Vec3 ColorBlack = { 0.0f, 0.0f, 0.0f };


struct WindowDimension {
    int width;
    int height;
};

struct Bitmap {
    size_t width;
    size_t height;
    void* pixels;   // 32 bit pixels, ARGB
};

#pragma pack(push, 1)
struct bitmap_header
{
    uint16_t file_type;
    uint32_t file_size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t bitmap_offset;
    uint32_t size;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bits_per_pixel;
    uint32_t compression;
    uint32_t size_of_bitmap;
    int32_t horizontal_resolution;
    int32_t vertical_resolution;
    uint32_t colors_used;
    uint32_t colors_important;

    uint32_t red_mask;
    uint32_t green_mask;
    uint32_t blue_mask;
};
#pragma pack(pop)

struct TextureEntry {
    char* key;
    GLuint id;
    Bitmap* bitmap;
};

enum SelectionState {
    SelectionState_None     = 0,
    SelectionState_Hover    = (1 << 0),
    SelectionState_Selected = (1 << 1),
};

enum ControlEvent {
    ControlEvent_None,
    ControlEvent_SetTargetPosition,
    ControlEvent_SetTargetHeading,
};

#define MAX_NUM_RENDER_GROUPS 512

struct RenderState {
    WindowDimension window_dimensions;
    Box last_imgui_window;
    bool mouse_buttons[3];
    Vec2 mouse_pos;

    ControlEvent control_event;
    uint32_t control_event_entity_index;
    Vec2 control_event_point;

    int scope_range;
    float feet_to_pixels;

    MemoryArena arena;

    RenderGroup* render_groups[MAX_NUM_RENDER_GROUPS];
    uint32_t num_render_groups;

    GLuint font_texture_id;
};

