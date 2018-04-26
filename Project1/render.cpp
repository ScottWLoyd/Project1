

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

static void set_color(Vec3 color)
{
    glColor3f(color.r, color.g, color.b);
}

static void draw_circle(float center_x, float center_y, float radius, Vec3 color, GLfloat line_width = 1.0f, bool fill = false)
{
    // TODO(scott): subpixel alignment?
    GLuint type = fill ? GL_TRIANGLE_FAN : GL_LINE_LOOP;
    glLineWidth(line_width);
    glBegin(type);
    set_color(color);
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
    RenderGroup* render_group = allocate_render_group(&state->render_state, &state->render_state.arena, entity, state);

    float ownship_heading = RADIANS(state->entities[state->ownship_index]->aircraft.heading);
    float slant_range = mag(entity->ned_pos);
    float bearing = atan2f(entity->ned_pos.e, entity->ned_pos.n);
    Vec2 center = vec2(slant_range*sinf(bearing - ownship_heading),
                       slant_range*cosf(bearing - ownship_heading));
    center *= state->render_state.feet_to_pixels;

    int shoot_list_priority = get_shoot_list_priority(state, entity);
    if (shoot_list_priority == 0)
    {
        RenderObject* lock_line = push_render_object(render_group, RenderObjectLine);
        lock_line->line.start = Vec2{ 0, 0 };
        lock_line->line.end = center;
        lock_line->line.line_width = 1;
        lock_line->color = ColorCyan;
    }
    if (shoot_list_priority >= 0)
    {
        RenderObject* circle;
        if (shoot_list_priority == 0)
        {
            circle = push_render_object(render_group, RenderObjectFillCircle);
        }
        else
        {
            circle = push_render_object(render_group, RenderObjectCircle);
        }
        circle->circle.center = center;
        circle->circle.radius = 23;
        circle->circle.line_width = (GLfloat)(entity->selected ? 2 : 1);
        circle->color = iff_status_to_color[entity->iff_status];
    }

    RenderObject* quad = push_render_object(render_group, RenderObjectTexturedRect);
    quad->textured_rect.center = center;
    quad->textured_rect.texture_id = aircraft_kind_to_texture_id(entity->aircraft.kind);

    Bitmap* bmp = get_bitmap(quad->textured_rect.texture_id);
    quad->textured_rect.dim = vec2((float)bmp->width, (float)bmp->height);
    quad->textured_rect.scale = vec2(1, 1);
    quad->textured_rect.rotation = rotation;
    if (shoot_list_priority == 0)
    {
        quad->color = ColorBlack;
    }
    else 
    {
        quad->color = iff_status_to_color[entity->iff_status];
    }
    if (entity->kind == EntityKind_Ownship)
    {
        quad->color = ColorBlue;
    }

    render_group->bounding.type = BoundingGeometry_Circle;
    render_group->bounding.circle = Circle{ center, 25 };
    render_group->pickable = true;    
}

static void add_static_render_objects(SimState* state)
{
    RenderState* render_state = &state->render_state;
    RenderGroup* group = allocate_render_group(render_state, &render_state->arena, NULL, state);
    // Outer range ring
    RenderObject* obj = push_render_object(group, RenderObjectCircle);
    obj->circle.center = vec2(0, 0);
    int min_dim = MIN(render_state->window_dimensions.width, render_state->window_dimensions.height);
    obj->circle.radius = (float)(min_dim / 2);
    obj->circle.line_width = 1;
    obj->color = ColorWhite;

    // Inner range ring
    obj = push_render_object(group, RenderObjectCircle);
    obj->circle.center = vec2(0, 0);
    obj->circle.radius = (float)(min_dim / 4);
    obj->circle.line_width = 1;
    obj->color = ColorWhite;

    // Scope Range 
    obj = push_render_object(group, RenderObjectText);
    sprintf(obj->text.text, "%d", (int)render_state->scope_range);
    obj->text.x = 50;
    obj->text.y = 50;

    // Compass
    obj = push_render_object(group, RenderObjectCompass);
    obj->compass.center = vec2(0, 0);
    obj->compass.radius = 0.25f;
    obj->compass.rotation = -state->entities[state->ownship_index]->aircraft.heading;
    obj->color = ColorWhite;
}

static void add_dynamic_render_objects(SimState* state)
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

static void handle_selection_processing(SimState* state)
{
    RenderState* render_state = &state->render_state;

    // Translate mouse position to the center of the screen like everything else
    Vec2 pos = render_state->mouse_pos - Vec2{ (float)(render_state->window_dimensions.width / 2), 
                                               (float)(render_state->window_dimensions.height / 2) };
    pos.y *= -1.0f;

    uint32_t selected_entity_index = 0;
    SelectionState selection_state = SelectionState_None;

    for (size_t index = 0; index < render_state->num_render_groups; index++)
    {
        RenderGroup* group = render_state->render_groups[index];
        if (group->pickable)
        {
            if (group->bounding.type == BoundingGeometry_Box)
            {
                if (box_contains_point(group->bounding.box, pos))
                {
                    selected_entity_index = group->entity_index;
                    selection_state = SelectionState_Hover;
                    break;
                }
            }
            else
            {
                assert(group->bounding.type == BoundingGeometry_Circle);
                if (circle_contains_point(group->bounding.circle, pos))
                {
                    selected_entity_index = group->entity_index;
                    selection_state = SelectionState_Hover;
                    break;
                }
            }
        }
    }

    for (uint32_t entity_index = 0; entity_index < state->num_entities; entity_index++)
    {
        EntityType* entity = state->entities[entity_index];
        if (entity_index == selected_entity_index)
        {
            entity->selected = true;
            entity->selection_state = selection_state;
        }
        else
        {
            entity->selected = false;
            entity->selection_state = SelectionState_None;
        }
    }
}

static void render(RenderState* state)
{
    glViewport(0, 0, state->window_dimensions.width, state->window_dimensions.height);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    float a = 2.0f / state->window_dimensions.width;
    float b = 2.0f / state->window_dimensions.height;
    float proj[] = {
        a,  0,  0,  0,
        0,  b,  0,  0,
        0,  0,  1,  0,
        -1, -1,  0,  1
    };
    glLoadMatrixf(proj);

    float min_dimension = (float)state->window_dimensions.height;
    if (state->window_dimensions.width < min_dimension)
    {
        min_dimension = (float)state->window_dimensions.width;
    }

    for (uint32_t render_index = 0; render_index < state->num_render_groups; render_index++)
    {
        RenderGroup* group = state->render_groups[render_index];
        for (uint32_t obj_index = 0; obj_index < group->num_render_objects; obj_index++)
        {
            RenderObject* object = group->render_objects + obj_index;
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
                    Vec2 center = { (state->window_dimensions.width * 0.5f) + object->textured_rect.center.x,
                                    (state->window_dimensions.height * 0.5f) + object->textured_rect.center.y };
                    glPushMatrix();
                    glTranslatef(center.x, center.y, 0);
                    float degrees = object->textured_rect.rotation;
                    glRotatef(degrees, 0, 0, 1);
                    Vec2 min_p = { -object->textured_rect.dim.x / 2, -object->textured_rect.dim.y / 2 };
                    Vec2 max_p = { object->textured_rect.dim.x / 2, object->textured_rect.dim.y / 2 };
                

                    glBegin(GL_QUADS);
                    set_color(object->color);
                    glTexCoord2f(0.0f, 0.0f); glVertex2f(min_p.x, min_p.y);
                    glTexCoord2f(1.0f, 0.0f); glVertex2f(max_p.x, min_p.y);
                    glTexCoord2f(1.0f, 1.0f); glVertex2f(max_p.x, max_p.y);                
                    glTexCoord2f(0.0f, 1.0f); glVertex2f(min_p.x, max_p.y);

                    glEnd();
                    glPopMatrix();
                
                    glDisable(GL_TEXTURE_2D);

                } break;

                case RenderObjectCircle: {
                
                    Vec2 center = { (state->window_dimensions.width * 0.5f) + object->circle.center.x,
                                    (state->window_dimensions.height * 0.5f) + object->circle.center.y };
                    draw_circle(center.x, center.y, object->circle.radius, object->color, object->circle.line_width);
                
                } break;

                case RenderObjectFillCircle: {

                    Vec2 center = { (state->window_dimensions.width * 0.5f) + object->circle.center.x,
                                    (state->window_dimensions.height * 0.5f) + object->circle.center.y };
                    draw_circle(center.x, center.y, object->circle.radius, object->color, object->circle.line_width, true);

                } break;

                case RenderObjectText: {
                
                    Vec2 center = { (state->window_dimensions.width * 0.5f) + object->textured_rect.center.x,
                                    (state->window_dimensions.height * 0.5f) + object->textured_rect.center.y };
                    float x = (state->window_dimensions.width * 0.5f) + (min_dimension * 0.4f);
                    float y = state->window_dimensions.height - 20.0f;
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
                    glDisable(GL_TEXTURE_2D);
                } break;

                case RenderObjectCompass: {
                
                    Vec2 center = { (state->window_dimensions.width * 0.5f) + object->compass.center.x,
                                    (state->window_dimensions.height * 0.5f) + object->compass.center.y };
                    float radius = 0.5f * min_dimension * object->compass.radius;
                    set_color(object->color);
                    glBegin(GL_LINES);
                    for (int degrees = 0; degrees < 360; degrees += 10)
                    {
                        float inner_scalar = 0.95f;
                        if (degrees % 90 == 0)
                        {
                            inner_scalar = 0.75f;
                        }
                        else if (degrees % 30 == 0)
                        {
                            inner_scalar = 0.85f;
                        }
                        float x1 = center.x + radius * sinf(RADIANS(degrees));
                        float y1 = center.y + radius * cosf(RADIANS(degrees));
                        float x2 = center.x + inner_scalar * radius * sinf(RADIANS(degrees));
                        float y2 = center.y + inner_scalar * radius * cosf(RADIANS(degrees));
                        glVertex2f(x1, y1);
                        glVertex2f(x2, y2);
                    }
                    glEnd();
                    glMatrixMode(GL_MODELVIEW);
                    glPushMatrix();
                    glTranslatef(center.x, center.y, 0);
                    glRotatef(-object->compass.rotation, 0, 0, 1);
                    set_color(ColorCyan);
                    glBegin(GL_TRIANGLES);
                    float x1 = - 0.07f * radius;
                    float y1 = radius;
                    float x2 = 0;
                    float y2 = 0.9f * radius;
                    float x3 = 0.07f * radius;
                    float y3 = radius;
                    glVertex2f(x1, y1);
                    glVertex2f(x2, y2);
                    glVertex2f(x3, y3);
                    glEnd();
                    glPopMatrix();

                } break;

                case RenderObjectLine: {

                    Vec2 center = { (state->window_dimensions.width * 0.5f),
                                    (state->window_dimensions.height * 0.5f) };
                    Vec2 start = center + object->line.start;
                    Vec2 end = center + object->line.end;
                    set_color(object->color);
                    glLineWidth(object->line.line_width);
                    glBegin(GL_LINES);
                    glVertex2f(start.x, start.y);
                    glVertex2f(end.x, end.y);
                    glEnd();

                } break;

                default: {
                    assert(!"Unhandled render object type");
                } break;
            } // switch (object->type)


        } // for (uint32_t obj_index = 0; obj_index < group->num_render_objects; obj_index++)
    } // for (uint32_t render_index = 0; render_index < state->num_render_groups; render_index++)
}
