

static void update_entity_position(SimState* state, EntityType* entity)
{
    // NOTE(scott): we typically want to update kinematics in ECEF
    // and then translate into geo and NED. Graphics are derived
    // from NED coordinates.
    entity->ned_pos = ecef_to_ned(state->entities[state->ownship_index]->geo_pos, entity->ecef_pos);
    entity->geo_pos = ecef_to_geo(entity->ecef_pos);
}

static void update_simulation(SimState* state)
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
        EntityType* ownship = state->entities[entity_index];
        set_ownship_geo_pos(ownship, vec3(0, 0, 30000));

        entity_index = add_entity(state, EntityKind_Aircraft);
        EntityType* entity = state->entities[entity_index];
        set_entity_ned_pos(entity, vec3(NM_TO_FT(28.3f), NM_TO_FT(28.3f), 0), ownship->geo_pos);
        set_entity_heading(entity, -135);
        entity->aircraft.kind = AircraftKind_SU35;
        entity->iff_status = IffStatusType_Hostile;
        add_to_shoot_list(state, entity);

        entity_index = add_entity(state, EntityKind_Aircraft);
        entity = state->entities[entity_index];
        set_entity_ned_pos(entity, vec3(NM_TO_FT(20), 0, 0), ownship->geo_pos);
        entity->aircraft.kind = AircraftKind_F22;
        entity->iff_status = IffStatusType_Friendly;
        add_to_shoot_list(state, entity);

        entity_index = add_entity(state, EntityKind_Aircraft);
        entity = state->entities[entity_index];
        set_entity_ned_pos(entity, vec3(0, NM_TO_FT(20), 0), ownship->geo_pos);
        entity->aircraft.kind = AircraftKind_F15;
        entity->iff_status = IffStatusType_Neutral;
        add_to_shoot_list(state, entity);

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
    if (state->time.effective_elapsed - last_one_hertz_time > 1.0f)
    {
        state->periodic.one_hertz = true;
        last_one_hertz_time = state->time.effective_elapsed;
    }
    if (state->time.effective_elapsed - last_five_hertz_time > 0.2f)
    {
        state->periodic.five_hertz = true;
        last_five_hertz_time = state->time.effective_elapsed;
    }
    if (state->time.effective_elapsed - last_ten_hertz_time > 0.1f)
    {
        state->periodic.ten_hertz = true;
        last_ten_hertz_time = state->time.effective_elapsed;
    }
    if (state->time.effective_elapsed - last_twenty_hertz_time > 0.05f)
    {
        state->periodic.twenty_hertz = true;
        last_twenty_hertz_time = state->time.effective_elapsed;
    }

    //
    // Four stages of the simulation:
    // 1. Unpack inputs from 1553/Ethernet
    // 2. Execute physics
    // 3. Process data
    // 4. Package outputs to 1553/Ethernet
    //

    EntityType* ownship = state->entities[state->ownship_index];
    for (uint32_t entity_index = 0; entity_index < state->num_entities; entity_index++)
    {
        EntityType* entity = state->entities[entity_index];

        switch (entity->kind)
        {
            case EntityKind_Ownship: {
                set_entity_heading(entity, entity->aircraft.heading + state->time.effective_elapsed * 5.0f);
            } break;

            case EntityKind_Aircraft: {

                if (entity->iff_status == IffStatusType_Hostile)
                {
                    float delta_pos = state->time.effective_elapsed * 1000.0f;
                    set_entity_ned_pos(entity, entity->ned_pos - vec3(delta_pos, delta_pos, 0), ownship->geo_pos);
                }
                else if (entity->iff_status == IffStatusType_Friendly)
                {
                    if (entity->aircraft.kind == AircraftKind_F22)
                    {
                        //float delta_hdg = state->dt * 10.0f;
                        //set_entity_heading(entity, entity->aircraft.heading + delta_hdg);
                    }
                }
                update_entity_position(state, entity);

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