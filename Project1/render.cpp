
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

        // ARGB
        int32_t red_shift = 16 - (int32_t)red_scan.index;
        int32_t green_shift = 8 - (int32_t)green_scan.index;
        int32_t blue_shift = 0 - (int32_t)blue_scan.index;
        int32_t alpha_shift = 24 - (int32_t)alpha_scan.index;

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

static TextureEntry cached_textures[20];
static size_t num_cached_textures = 0;

static void CreateTexture(char* file_path, char* key)
{
    for (size_t texture_index=0; texture_index < num_cached_textures; texture_index++)
    {
        TextureEntry* it = &cached_textures[texture_index];
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)entry.bitmap->width, (GLsizei)entry.bitmap->height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, (void*)entry.bitmap->pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    
    assert(num_cached_textures < ArrayCount(cached_textures));
    cached_textures[num_cached_textures++] = entry;
}

static GLuint get_texture_id(char* key)
{
    for (size_t texture_index = 0; texture_index < num_cached_textures; texture_index++)
    {
        TextureEntry* it = &cached_textures[texture_index];
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
    for (size_t texture_index = 0; texture_index < num_cached_textures; texture_index++)
    {
        TextureEntry* it = &cached_textures[texture_index];
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
    RenderObject* obj = (RenderObject*)push_struct(&state->arena, RenderObject);
    obj->type = type;

    state->render_objects[state->num_render_objects++] = obj;

    return obj;
}

static int get_desired_pixel_format(HDC device_context)
{
    PIXELFORMATDESCRIPTOR pfd = { 0 };
    int pixel_format_count = DescribePixelFormat(device_context, 1, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

    // We need to find an ICD OpenGL pixel format, we don't care about any of the
    // other specifics...
    for (int pixel_format = 0; pixel_format < pixel_format_count; pixel_format++) 
    {
        zero_struct(pfd);
        DescribePixelFormat(device_context, pixel_format, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

        if (!(pfd.dwFlags & PFD_SUPPORT_OPENGL))
            continue;

        // ICD pixel formats aren't generic (GDI), nor are they generic accelerated (MCD)
        if (!((pfd.dwFlags & PFD_GENERIC_FORMAT) || (pfd.dwFlags & PFD_GENERIC_ACCELERATED)))
            return pixel_format;
    }

    // No ICD Pixel Format Found?!
    return 0;
}

#define WGL_DRAW_TO_WINDOW_ARB 0x2001
#define WGL_SUPPORT_OPENGL_ARB 0x2010
#define WGL_DOUBLE_BUFFER_ARB  0x2011
#define WGL_PIXEL_TYPE_ARB     0x2013
#define WGL_TYPE_RGBA_ARB      0x202B
#define WGL_COLOR_BITS_ARB     0x2014
#define WGL_SAMPLE_BUFFERS_ARB 0x2041
#define WGL_SAMPLES_ARB        0x2042
#define GL_MULTISAMPLE         0x809D 

#define MAX_FORMATS 20
typedef const char *(WINAPI * PWGLGETEXTENSIONSSTRING)(HDC hdc); 
typedef bool(*PWGLCHOOSEPIXELFORMATARB)(HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);

static HWND InitOpenGL(HINSTANCE hInstance)
{
    HWND window = CreateWindow("Project1 Window Class", "Dummy Window",
        WS_DISABLED, 0, 0, 0, 0, NULL, NULL, hInstance, NULL);

    HDC device_context = GetDC(window);

    PIXELFORMATDESCRIPTOR pixel_format = { 0 };
    pixel_format.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pixel_format.nVersion = 1;
    
    SetPixelFormat(device_context, get_desired_pixel_format(device_context), &pixel_format);

    // If we fail, don't bother trying to initialize any extensions...
    HGLRC opengl_rc = wglCreateContext(device_context);
    if (!opengl_rc)
    {
        // TODO(scott): logging...
        DestroyWindow(window);
        assert(false);
    }
    
    if (!wglMakeCurrent(device_context, opengl_rc))
    {
        // TODO(scott): logging...
        DestroyWindow(window);
        assert(false);
    }

    // load extensions
    PWGLGETEXTENSIONSSTRING wglGetExtensionsStringARB = (PWGLGETEXTENSIONSSTRING)wglGetProcAddress("wglGetExtensionsStringARB");
    if (!wglGetExtensionsStringARB)
    {
        // TODO(scott): logging...
        assert(false);
    }
    const GLubyte* extensions = (const GLubyte*)wglGetExtensionsStringARB(device_context);
        
    PWGLCHOOSEPIXELFORMATARB wglChoosePixelFormatARB = (PWGLCHOOSEPIXELFORMATARB)wglGetProcAddress("wglChoosePixelFormatARB");
    if (!wglChoosePixelFormatARB)
    {
        // TODO(scott): logging...
        assert(false);
    }

    // init the pixel format
    int attribs[] = {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
        WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB, 32,
        WGL_SAMPLE_BUFFERS_ARB, 1, // Number of buffers (must be 1 at time of writing)
        WGL_SAMPLES_ARB, 4,        // Number of samples
        0, // End
    };
    int formats[MAX_FORMATS];
    uint32_t num_formats;
    wglChoosePixelFormatARB(device_context, attribs, NULL, MAX_FORMATS, formats, &num_formats);

    // Cleanup dummy context
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(opengl_rc);
    DestroyWindow(window);


    // Create the actual window & context
    window = CreateWindow("Project1 Window Class", "Project1 Window",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        0, 0, hInstance, 0);

    device_context = GetDC(window);
    // TODO(scott): check for a match on the pixel format here
    SetPixelFormat(device_context, formats[0], &pixel_format);

    // If we fail, don't bother trying to initialize any extensions...
    opengl_rc = wglCreateContext(device_context);
    if (!opengl_rc)
    {
        // TODO(scott): logging...
        DestroyWindow(window);
        assert(false);
    }

    if (!wglMakeCurrent(device_context, opengl_rc))
    {
        // TODO(scott): logging...
        DestroyWindow(window);
        assert(false);
    }
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);

    return window;
}
#undef WGL_DRAW_TO_WINDOW_ARB 
#undef WGL_SUPPORT_OPENGL_ARB 
#undef WGL_DOUBLE_BUFFER_ARB  
#undef WGL_PIXEL_TYPE_ARB     
#undef WGL_TYPE_RGBA_ARB      
#undef WGL_COLOR_BITS_ARB     
#undef WGL_SAMPLE_BUFFERS_ARB 
#undef WGL_SAMPLES_ARB        
#undef GL_MULTISAMPLE
#undef MAX_FORMATS


static stbtt_bakedchar cdata[96]; // ASCII 32..126 is 95 glyphs

static void init_render_state(RenderState* state)
{
    unsigned char* ttf_buffer = (unsigned char*)xmalloc(1 << 20);
    unsigned char* temp_bitmap = (unsigned char*)xmalloc(512 * 512);
    fread(ttf_buffer, 1, 1 << 20, fopen("c:/windows/fonts/arial.ttf", "rb"));
    stbtt_BakeFontBitmap(ttf_buffer, 0, 32.0, temp_bitmap, 512, 512, 32, 96, cdata); // no guarantee this fits!
                                                                                     // can free ttf_buffer at this point
    free(ttf_buffer);

    glGenTextures(1, &state->font_texture_id);
    glBindTexture(GL_TEXTURE_2D, state->font_texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 512, 512, 0, GL_ALPHA, GL_UNSIGNED_BYTE, temp_bitmap);
    // can free temp_bitmap at this point
    free(temp_bitmap);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    // Load textures
    CreateTexture("../res/f-15-small.bmp", "f-15");
    CreateTexture("../res/f-22-small.bmp", "f-22");
    CreateTexture("../res/su-35-small.bmp", "su-35");

    state->scope_range = 40;
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

static void add_aircraft_render_object(SimState* state, EntityType* entity, float rotation)
{
    RenderObject* quad = push_render_object(&state->render_state, RenderObjectTexturedRect);

    // TODO(scott): convert ECEF or NED coordinates to screen space
    quad->textured_rect.center = vec2(entity->pos.x, entity->pos.y);
    quad->textured_rect.texture_id = aircraft_kind_to_texture_id(entity->aircraft.kind);

    Bitmap* bmp = get_bitmap(quad->textured_rect.texture_id);
    quad->textured_rect.dim = vec2((float)bmp->width, (float)bmp->height);
    quad->textured_rect.scale = vec2(1, 1);
    quad->textured_rect.rotation = rotation;
    quad->color = iff_status_to_color[entity->iff_status];
    if (entity->kind == EntityKind_Ownship)
    {
        quad->color = ColorBlue;
    }
}

static void add_static_render_objects(RenderState* state)
{
    // Outer range ring
    RenderObject* obj = push_render_object(state, RenderObjectCircle);
    obj->circle.center = vec2(0, 0);
    obj->circle.radius = 1.0f;
    obj->color = ColorWhite;

    obj = push_render_object(state, RenderObjectText);
    sprintf(obj->text.text, "%d", (int)state->scope_range);
    obj->text.x = 50;
    obj->text.y = 50;
}

static void add_dynamic_render_objects(SimState* state, WindowDimension dimensions)
{
    EntityType* ownship = NULL;
    for (uint32_t entity_index = 0; entity_index < state->num_entities; entity_index++)
    {
        EntityType* entity = state->entities[entity_index];
        switch (entity->kind)
        {
            case EntityKind_Ownship: {
                ownship = state->entities[entity_index];
                add_aircraft_render_object(state, entity, 0);
            } break;

            case EntityKind_Aircraft: {
                float rotation = -(entity->aircraft.heading - ownship->aircraft.heading);
                add_aircraft_render_object(state, entity, rotation);
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
                // NOTE(scott): for pre-multiplied alpha
                //glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

                glBindTexture(GL_TEXTURE_2D, object->textured_rect.texture_id);

                glMatrixMode(GL_MODELVIEW);
                Vec2 center = { (dimensions.width * 0.5f) + object->textured_rect.center.x, 
                                (dimensions.height * 0.5f) + object->textured_rect.center.y };
                glPushMatrix();
                glTranslatef(center.x, center.y, 0);
                float degrees = object->textured_rect.rotation;
                glRotatef(degrees, 0, 0, 1);
                Vec2 min_p = { -object->textured_rect.dim.x / 2, -object->textured_rect.dim.y / 2 };
                Vec2 max_p = { object->textured_rect.dim.x / 2, object->textured_rect.dim.y / 2 };
                

                glBegin(GL_QUADS);
                glColor3f(object->color.r, object->color.g, object->color.b);
                glTexCoord2f(0.0f, 0.0f); glVertex2f(min_p.x, min_p.y);
                glTexCoord2f(1.0f, 0.0f); glVertex2f(max_p.x, min_p.y);
                glTexCoord2f(1.0f, 1.0f); glVertex2f(max_p.x, max_p.y);                
                glTexCoord2f(0.0f, 1.0f); glVertex2f(min_p.x, max_p.y);

                glEnd();
                glPopMatrix();
                
                glDisable(GL_TEXTURE_2D);

            } break;

            case RenderObjectCircle: {
                
                Vec2 center = { (dimensions.width * 0.5f) + object->circle.center.x,
                    (dimensions.height * 0.5f) + object->circle.center.y };
                float radius = 0.5f * min_dimension * object->circle.radius;
                draw_circle((int)center.x, (int)center.y, radius, object->color);
                
            } break;

            case RenderObjectText: {
                
                Vec2 center = { (dimensions.width * 0.5f) + object->textured_rect.center.x,
                    (dimensions.height * 0.5f) + object->textured_rect.center.y };
                float x = (dimensions.width * 0.5f) + (min_dimension * 0.4f);
                float y = dimensions.height - 20.0f;
                // origin at center, units in screen pixels
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, state->font_texture_id);
                glBegin(GL_QUADS);
                char* text = object->text.text;
                while (*text) {
                    if (*text >= 32 && *text < 128) {
                        stbtt_aligned_quad q;
                        stbtt_GetBakedQuad(cdata, 512, 512, *text - 32, &x, &y, &q, 1);//1=opengl & d3d10+,0=d3d9
                        glTexCoord2f(q.s0, q.t1); glVertex2f(q.x0, q.y0);
                        glTexCoord2f(q.s1, q.t1); glVertex2f(q.x1, q.y0);
                        glTexCoord2f(q.s1, q.t0); glVertex2f(q.x1, q.y1);
                        glTexCoord2f(q.s0, q.t0); glVertex2f(q.x0, q.y1);
                    }
                    ++text;
                }
                glEnd();
            } break;

            default: {
                assert(!"Unhandled render object type");
            } break;
        }
    }

    state->arena.next = state->arena.base;
    state->num_render_objects = 0;    
}
