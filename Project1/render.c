Vec3 ColorRed = { 1.0f, 0.0f, 0.0f };
Vec3 ColorGreen = { 0.0f, 1.0f, 0.0f };
Vec3 ColorBlue = { 0.0f, 0.0f, 1.0f };
Vec3 ColorMagenta = { 1.0f, 0.0f, 1.0f };
Vec3 ColorYellow = { 1.0f, 1.0f, 0.0f };
Vec3 ColorCyan = { 0.0f, 1.0f, 1.0f };
Vec3 ColorWhite = { 1.0f, 1.0f, 1.0f };
Vec3 ColorBlack = { 0.0f, 0.0f, 0.0f };


#pragma pack(push, 1)
typedef struct DibHeader {
    uint32_t header_size;
    uint32_t bitmap_width;
    uint32_t bitmap_height;
    uint16_t color_planes;
    uint16_t bits_per_pixel;
    uint32_t compression;
    uint32_t image_size;
    int32_t horizontal_resolution; // pixel/meter
    int32_t vertical_resolution; // pixel/meter
    uint32_t num_colors;
    uint32_t important_colors;
} DibHeader;

typedef struct BitmapHeader {
    char bmp_identifier[2];
    uint32_t size_in_bytes;
    char reserved[4];
    uint32_t pixel_offset;
} BitmapHeader;

typedef struct Bitmap {
    size_t width;
    size_t height;
    void* pixels;   // 32 bit pixels, ARGB
} Bitmap;
#pragma pack(pop)

static Bitmap CrossLoadBitmap(char* file_path)
{
    Bitmap result = { 0 };
    FileContents contents = ReadEntireFile(file_path);
    if (contents.success)
    {
        char* ptr = contents.data;
        BitmapHeader* header = (BitmapHeader*)ptr;
        ptr += sizeof(BitmapHeader);
        DibHeader* dib_header = (DibHeader*)ptr;

        ptr += dib_header->header_size;

        result.width = dib_header->bitmap_width;
        result.height = dib_header->bitmap_height;
        result.pixels = ptr;
    }

    return result;
}

typedef struct TextureEntry {
    char* key;
    GLuint id;
    Bitmap* bitmap;
} TextureEntry;

TextureEntry* cached_textures;

static void CreateTexture(char* file_path, char* key)
{
    for (TextureEntry* it = cached_textures; it != buf_end(cached_textures); it++)
    {
        if (strcmp(it->key, key) == 0)
        {
            assert(!"Key already exists for texture!");
        }
    }

    TextureEntry entry;
    Bitmap bitmap = CrossLoadBitmap(file_path);
    entry.bitmap = &bitmap;
    entry.key = key;
    
    glGenTextures(1, &entry.id);
    glBindTexture(GL_TEXTURE_2D, entry.id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, entry.bitmap->width, entry.bitmap->height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, entry.bitmap->pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    
    buf_push(cached_textures, entry);
}

static GLuint get_texture_id(char* key)
{
    for (TextureEntry* it = cached_textures; it != buf_end(cached_textures); it++)
    {
        if (strcmp(it->key, key) == 0)
        {
            return it->id;
        }
    }
    assert(!"Texture not found!");
    return -1;
}

typedef enum RenderObjectType {
    RenderObjectTexturedRect,
    RenderObjectCircle,
    RenderObjectLine,
} RenderObjectType;

typedef struct RenderObject {
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
    };
} RenderObject;

#define MAX_NUM_RENDER_OBJECTS 1024

typedef struct RenderState {
    MemoryArena arena;

    RenderObject* render_objects[MAX_NUM_RENDER_OBJECTS];
    uint32_t num_render_objects;
} RenderState;

static RenderObject* push_render_object(RenderState* state, RenderObjectType type)
{
    assert(state->num_render_objects < MAX_NUM_RENDER_OBJECTS);
    RenderObject* obj = push_struct(&state->arena, RenderObject);
    obj->type = type;

    state->render_objects[state->num_render_objects++] = obj;

    return obj;
}

static void InitOpenGL(HWND window)
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

    // Load textures
    CreateTexture("./res/target.bmp", "target");

    glEnable(GL_TEXTURE_2D);
}


static void draw_circle(int center_x, int center_y, float radius, Vec3 color)
{
    // TODO(scott): subpixel alignment?
    glBegin(GL_LINE_LOOP);
    glColor3f(color.r, color.g, color.b);
    // TODO(scott): determine the number of points based on size of circle / resolution
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

static void AddStaticRenderObjects(RenderState* state)
{
    // Outer range ring
    RenderObject* obj = push_render_object(state, RenderObjectCircle);
    obj->circle.center = vec2(0, 0);
    obj->circle.radius = 1.0f;
    obj->color = ColorWhite;
}

typedef struct WindowDimension {
    int width;
    int height;
} WindowDimension;

static void Render(RenderState* state, WindowDimension dimensions)
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

    float min_dimension = (float)dimensions.height;
    if (dimensions.width < min_dimension)
    {
        min_dimension = (float)dimensions.width;
    }

    for (uint32_t render_index = 0; render_index < state->num_render_objects; render_index++)
    {
        RenderObject* object = state->render_objects[render_index];
        switch (object->type)
        {
            case RenderObjectTexturedRect: {
                
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, object->textured_rect.texture_id);

                glBegin(GL_TRIANGLES);

                Vec2 center = { (dimensions.width * 0.5f) + object->textured_rect.center.x, 
                                (dimensions.height * 0.5f) + object->textured_rect.center.y };
                Vec2 min_p = { center.x - object->textured_rect.dim.x / 2, center.y - object->textured_rect.dim.y / 2 };
                Vec2 max_p = { center.x + object->textured_rect.dim.x / 2, center.y + object->textured_rect.dim.y / 2 };

                glColor3f(object->color.r, object->color.g, object->color.b);
                glTexCoord2f(0.0f, 0.0f);
                glVertex2f(min_p.x, min_p.y);
                glTexCoord2f(1.0f, 0.0f);
                glVertex2f(max_p.x, min_p.y);
                glTexCoord2f(1.0f, 1.0f);
                glVertex2f(max_p.x, max_p.y);

                glTexCoord2f(0.0f, 0.0f);
                glVertex2f(min_p.x, min_p.y);
                glTexCoord2f(1.0f, 1.0f);
                glVertex2f(max_p.x, max_p.y);
                glTexCoord2f(0.0f, 1.0f);
                glVertex2f(min_p.x, max_p.y);

                glEnd();
                
                glDisable(GL_TEXTURE_2D);

            } break;

            case RenderObjectCircle: {
                
                Vec2 center = { (dimensions.width * 0.5f) + object->circle.center.x,
                    (dimensions.height * 0.5f) + object->circle.center.y };
                float radius = 0.5f * min_dimension * object->circle.radius;
                draw_circle(center.x, center.y, radius, object->color);
                
            } break;

            default: {
                assert(!"Unhandled render object type");
            } break;
        }
    }

    state->arena.next = state->arena.base;
    state->num_render_objects = 0;    
}
