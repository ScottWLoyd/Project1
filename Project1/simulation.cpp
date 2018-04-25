
static uint32_t add_entity(SimState* state, EntityKind kind)
{
    EntityType* entity = (EntityType*)push_struct(&state->sim_arena, EntityType);
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

        entity_index = add_entity(state, EntityKind_Aircraft);
        state->entities[entity_index]->pos = vec3(0, 150, 0);
        state->entities[entity_index]->aircraft.kind = AircraftKind_F15;
        state->entities[entity_index]->iff_status = IffStatusType_Neutral;

        iff_status_to_color[IffStatusType_None] = ColorWhite;
        iff_status_to_color[IffStatusType_Friendly] = ColorGreen;
        iff_status_to_color[IffStatusType_Neutral] = ColorYellow;
        iff_status_to_color[IffStatusType_Hostile] = ColorRed;

        state->initialized = true;
    }

    //
    // Calculate the periodic flags
    //
    static float last_one_hertz_time = 0.0f;
    static float last_five_hertz_time = 0.0f;
    static float last_ten_hertz_time = 0.0f;
    static float last_twenty_hertz_time = 0.0f;
    if (state->dt - last_one_hertz_time > 1.0f)
    {
        state->periodic.one_hertz = true;
        last_one_hertz_time = state->dt;
    }
    if (state->dt - last_five_hertz_time > 0.2f)
    {
        state->periodic.five_hertz = true;
        last_five_hertz_time = state->dt;
    }
    if (state->dt - last_ten_hertz_time > 0.1f)
    {
        state->periodic.ten_hertz = true;
        last_ten_hertz_time = state->dt;
    }
    if (state->dt - last_twenty_hertz_time > 0.05f)
    {
        state->periodic.twenty_hertz = true;
        last_twenty_hertz_time = state->dt;
    }

    //
    // Four stages of the simulation:
    // 1. Unpack inputs from 1553/Ethernet
    // 2. Execute physics
    // 3. Process data
    // 4. Package outputs to 1553/Ethernet
    //

    for (uint32_t entity_index = 0; entity_index < state->num_entities; entity_index++)
    {
        EntityType* entity = state->entities[entity_index];

        switch (entity->kind)
        {
            case EntityKind_Ownship: {
                set_entity_heading(entity, 90.0f);
            } break;

            case EntityKind_Aircraft: {

                if (entity->iff_status == IffStatusType_Hostile)
                {
                    float delta_pos = state->dt * 10.0f;
                    set_entity_pos(entity, vec3_sub(entity->pos, vec3(delta_pos, delta_pos, 0)));
                }
                else if (entity->iff_status == IffStatusType_Friendly)
                {
                    if (entity->aircraft.kind == AircraftKind_F22)
                    {
                        float delta_hdg = state->dt * 10.0f;
                        set_entity_heading(entity, entity->aircraft.heading + delta_hdg);
                    }
                }

            } break;

            default: {
            } break;
        }
    }

    state->periodic.one_hertz = false;
    state->periodic.five_hertz = false;
    state->periodic.ten_hertz = false;
    state->periodic.twenty_hertz = false;
}