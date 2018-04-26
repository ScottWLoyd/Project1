
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
    for (size_t texture_index = 0; texture_index < num_cached_textures; texture_index++)
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

static RenderObject* push_render_object(RenderGroup* group, RenderObjectType type)
{
    assert(group->num_render_objects < group->max_render_objects);
    RenderObject* obj = group->render_objects + group->num_render_objects++;
    obj->type = type;
    return obj;
}

static RenderGroup* allocate_render_group(RenderState* state, MemoryArena* arena, EntityType* entity, SimState* sim_state, size_t max_capacity = 8)
{
    assert(state->num_render_groups < ArrayCount(state->render_groups));
    RenderGroup* group = push_struct(arena, RenderGroup);
    state->render_groups[state->num_render_groups++] = group;

    group->render_objects = (RenderObject*)push_size(arena, sizeof(RenderObject) * max_capacity);
    group->max_render_objects = max_capacity;
    group->num_render_objects = 0;
    group->entity_index = get_entity_index(sim_state, entity);
    group->pickable = false;    

    return group;
}