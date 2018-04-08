
static uint32_t add_entity(SimState* state, EntityKind kind)
{
    EntityType* entity = push_struct(&state->sim_arena, sizeof(EntityType));
    zero_struct(*entity);
    entity->kind = kind;

    uint32_t index = state->num_entities;
    state->entities[state->num_entities++] = entity;

    return index;
}

static void UpdateSimulation(SimState* state)
{
    if (!state->initialized)
    {
        // Null entity
        uint32_t entity_index = add_entity(state, EntityKind_None);

        entity_index = add_entity(state, EntityKind_Ownship);

        state->initialized = true;
    }

    for (uint32_t entity_index = 0; entity_index < state->num_entities; entity_index++)
    {
        EntityType* entity = state->entities[entity_index];

        switch (entity->kind)
        {
            case EntityKind_Ownship: {


                RenderObject* quad = push_render_object(&state->render_state, RenderObjectTexturedRect);
                quad->textured_rect.center = vec2(0, 0);
                quad->textured_rect.dim = vec2(50, 50);
                quad->textured_rect.scale = vec2(1, 1);
                quad->color = ColorCyan;
                quad->textured_rect.texture_id = get_texture_id("target");

            } break;

            default: {

            } break;
        }
    }
}