
static uint32_t add_entity(SimState* state, EntityKind kind)
{
    EntityType* entity = push_struct(&state->sim_arena, EntityType);
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
        // Reset
        state->sim_arena.next = state->sim_arena.base;
        state->num_entities = 0;
        state->ownship_index = 0;

        // Null entity
        uint32_t entity_index = add_entity(state, EntityKind_None);

        entity_index = add_entity(state, EntityKind_Ownship);
        state->ownship_index = entity_index;

        entity_index = add_entity(state, EntityKind_Aircraft);
        state->entities[entity_index]->pos = vec3(150, 150, 0);
        state->entities[entity_index]->aircraft.kind = AircraftKind_SU35;
        state->entities[entity_index]->iff_status = IffStatusType_Hostile;

        entity_index = add_entity(state, EntityKind_Aircraft);
        state->entities[entity_index]->pos = vec3(-150, 150, 0);
        state->entities[entity_index]->aircraft.kind = AircraftKind_F22;
        state->entities[entity_index]->iff_status = IffStatusType_Friendly;

        iff_status_to_color[IffStatusType_None] = ColorWhite;
        iff_status_to_color[IffStatusType_Friendly] = ColorGreen;
        iff_status_to_color[IffStatusType_Neutral] = ColorYellow;
        iff_status_to_color[IffStatusType_Hostile] = ColorRed;

        state->initialized = true;
    }

    for (uint32_t entity_index = 0; entity_index < state->num_entities; entity_index++)
    {
        EntityType* entity = state->entities[entity_index];

        switch (entity->kind)
        {
            case EntityKind_Ownship: {

            } break;

            case EntityKind_Aircraft: {

                if (entity->iff_status == IffStatusType_Hostile)
                {
                    float delta_pos = state->dt * 10.0f;
                    set_entity_pos(entity, vec3_sub(entity->pos, vec3(delta_pos, delta_pos, 0)));
                }

            } break;

            default: {
            } break;
        }
    }
}