
static Bitmap* CrossLoadBitmap(char* file_path)
{
    Bitmap* result = (Bitmap*)calloc(1, sizeof(Bitmap));

    FileContents read_result = ReadEntireFile(file_path);
    if (read_result.len != 0)
    {
        bitmap_header *header = (bitmap_header *)read_result.data;
        uint32_t *pixels = (uint32_t *)((uint8_t *)read_result.data + header->bitmap_offset);
        result->pixels = pixels;
        result->width = header->width;
        result->height = header->height;

        assert(header->compression == 3);

        uint32_t red_mask = header->red_mask;
        uint32_t green_mask = header->green_mask;
        uint32_t blue_mask = header->blue_mask;
        uint32_t alpha_mask = ~(red_mask | green_mask | blue_mask);

        BitScanResult red_scan = find_least_significant_set_bit(red_mask);
        BitScanResult green_scan = find_least_significant_set_bit(green_mask);
        BitScanResult blue_scan = find_least_significant_set_bit(blue_mask);
        BitScanResult alpha_scan = find_least_significant_set_bit(alpha_mask);

        assert(red_scan.found);
        assert(green_scan.found);
        assert(blue_scan.found);
        assert(alpha_scan.found);

#if 1
        // ARGB
        int32_t red_shift = 16 - (int32_t)red_scan.index;
        int32_t green_shift = 8 - (int32_t)green_scan.index;
        int32_t blue_shift = 0 - (int32_t)blue_scan.index;
        int32_t alpha_shift = 24 - (int32_t)alpha_scan.index;
#else
        // BGRA
        int32_t red_shift = 8 - (int32_t)red_scan.index;
        int32_t green_shift = 16 - (int32_t)green_scan.index;
        int32_t blue_shift = 24 - (int32_t)blue_scan.index;
        int32_t alpha_shift = 0 - (int32_t)alpha_scan.index;
#endif

        uint32_t *source_dest = pixels;
        for (int32_t y = 0; y < header->height; y++)
        {
            for (int32_t x = 0; x < header->width; x++)
            {
                uint32_t color = *source_dest;

                *source_dest++ = (rotate_left(color & red_mask, red_shift) |
                    rotate_left(color & green_mask, green_shift) |
                    rotate_left(color & blue_mask, blue_shift) |
                    rotate_left(color & alpha_mask, alpha_shift));
            }
        }
    }

    return result;
}
#if 0
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

typedef struct DibHeaderV4 {
    uint32_t header_size;
    int32_t bitmap_width;
    int32_t bitmap_height;
    uint16_t color_planes;
    uint16_t bits_per_pixel;
    uint32_t compression;
    uint32_t image_size;
    int32_t horizontal_resolution; // pixel/meter
    int32_t vertical_resolution; // pixel/meter
    uint32_t num_colors;
    uint32_t important_colors;
    uint32_t red_mask;
    uint32_t green_mask;
    uint32_t blue_mask;
    uint32_t alpha_mask;
    uint32_t cs_type;
    int endpoints;
    uint32_t gamma_red;
    uint32_t gamma_green;
    uint32_t gamma_blue;
} DibHeaderV4;

typedef struct DibHeaderV5 {
    uint32_t header_size;
    int32_t bitmap_width;
    int32_t bitmap_height;
    uint16_t color_planes;
    uint16_t bits_per_pixel;
    uint32_t compression;
    uint32_t image_size;
    int32_t horizontal_resolution; // pixel/meter
    int32_t vertical_resolution; // pixel/meter
    uint32_t num_colors;
    uint32_t important_colors;
    uint32_t red_mask;
    uint32_t green_mask;
    uint32_t blue_mask;
    uint32_t alpha_mask;
    uint32_t cs_type;
    int endpoints;
    uint32_t gamma_red;
    uint32_t gamma_green;
    uint32_t gamma_blue;
    uint32_t intent;
    uint32_t profile_data;
    uint32_t profile_size;
    uint32_t reserved;
} DibHeaderV5;

typedef struct BitmapHeader {
    char bmp_identifier[2];
    uint32_t size_in_bytes;
    uint16_t reserved[2];
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
        uint32_t size = *(uint32_t*)ptr;

        DibHeader* dib_header = (DibHeader*)ptr;
        result.width = dib_header->bitmap_width;
        result.height = dib_header->bitmap_height;

        assert(header->pixel_offset == (sizeof(BitmapHeader) + dib_header->header_size));
        
        ptr = (char*)contents.data + header->pixel_offset;
        result.pixels = ptr;
    }

    return result;
}
#endif

static TextureEntry* cached_textures;

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
    Bitmap* bitmap = CrossLoadBitmap(file_path);
    entry.bitmap = bitmap;
    entry.key = key;
    
    glGenTextures(1, &entry.id);
    glBindTexture(GL_TEXTURE_2D, entry.id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, entry.bitmap->width, entry.bitmap->height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, (void*)entry.bitmap->pixels);

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

static Bitmap* get_bitmap(GLuint texture_id)
{
    for (TextureEntry* it = cached_textures; it != buf_end(cached_textures); it++)
    {
        if (it->id == texture_id)
        {
            return it->bitmap;
        }
    }
    return NULL;
}

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
    CreateTexture("../res/f-15-small.bmp", "f-15");
    CreateTexture("../res/f-22-small.bmp", "f-22");
    CreateTexture("../res/su-35-small.bmp", "su-35");

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
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


static GLuint aircraft_kind_to_texture_id(AircraftKind kind)
{
    if (kind == AircraftKind_F15)
        return get_texture_id("f-15");
    else if (kind == AircraftKind_F22)
        return get_texture_id("f-22");
    else if (kind == AircraftKind_SU35)
        return get_texture_id("su-35");
    else
        assert(!"Unrecognized aircraft kind!");
    return 0;
}

static Vec3 iff_status_to_color[IffStatusType_Count];

static void add_aircraft_render_object(SimState* state, uint32_t entity_index)
{
    EntityType* entity = state->entities[entity_index];
    RenderObject* quad = push_render_object(&state->render_state, RenderObjectTexturedRect);

    // TODO(scott): convert ECEF or NED coordinates to screen space
    quad->textured_rect.center = vec2(entity->pos.x, entity->pos.y);
    quad->textured_rect.texture_id = aircraft_kind_to_texture_id(entity->aircraft.kind);

    Bitmap* bmp = get_bitmap(quad->textured_rect.texture_id);
    quad->textured_rect.dim = vec2(bmp->width, bmp->height);
    quad->textured_rect.scale = vec2(1, 1);
    quad->color = iff_status_to_color[entity->iff_status];
    if (entity_index == state->ownship_index)
    {
        quad->color = ColorCyan;
    }
}

static void add_static_render_objects(RenderState* state)
{
    // Outer range ring
    RenderObject* obj = push_render_object(state, RenderObjectCircle);
    obj->circle.center = vec2(0, 0);
    obj->circle.radius = 1.0f;
    obj->color = ColorWhite;
}

static void add_dynamic_render_objects(SimState* state, WindowDimension dimensions)
{
    for (uint32_t entity_index = 0; entity_index < state->num_entities; entity_index++)
    {
        switch (state->entities[entity_index]->kind)
        {
            case EntityKind_Ownship:
            case EntityKind_Aircraft: {

                add_aircraft_render_object(state, entity_index);

            } break;

            default: {
            } break;
        }
    }
}

static void render(RenderState* state, WindowDimension dimensions)
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
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                //glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

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
