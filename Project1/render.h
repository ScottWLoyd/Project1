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

enum RenderObjectType {
    RenderObjectTexturedRect,
    RenderObjectCircle,
    RenderObjectLine,
    RenderObjectText,
};

struct RenderObject {
    RenderObjectType type;
    Vec3 color;
    union {
        struct {
            GLuint texture_id;
            Vec2 center;
            Vec2 dim;
            float rotation;
            Vec2 scale;
        } textured_rect;
        struct {
            Vec2 center;
            float radius;
        } circle;
        struct {
            Vec2 start;
            Vec2 end;
        } line;
        struct {
            char text[20];
            float size;
            float x;
            float y;
        } text;
    };
};

#define MAX_NUM_RENDER_OBJECTS 1024

struct RenderState {
    int scope_range;
    MemoryArena arena;

    RenderObject* render_objects[MAX_NUM_RENDER_OBJECTS];
    uint32_t num_render_objects;

    GLuint font_texture_id;

};

