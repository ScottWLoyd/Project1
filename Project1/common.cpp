

static uint32_t add_entity(SimState* state, EntityKind kind)
{
    uint32_t index = state->num_entities;
    EntityType* entity = state->entities + state->num_entities++;
    zero_struct(*entity);
    entity->kind = kind;

    return index;
}

static void remove_entity(SimState* state, uint32_t entity_index)
{
    state->entities[entity_index] = state->entities[state->num_entities - 1];
    state->num_entities--;
}

#define set_ownship_geo_pos(e, p) set_entity_geo_pos(e, p, p)

static void set_entity_geo_pos(EntityType* entity, Vec3 new_geo_pos, Vec3 ownship_geo_pos)
{
    entity->pos.geo = new_geo_pos;
    entity->pos.ecef = geo_to_ecef(entity->pos.geo);
    entity->pos.ned = ecef_to_ned(ownship_geo_pos, entity->pos.ecef);
}

static void set_entity_ned_pos(EntityType* entity, Vec3 new_ned_pos, Vec3 ownship_geo_pos)
{
    entity->pos.ned = new_ned_pos;
    entity->pos.ecef = ned_to_ecef(ownship_geo_pos, new_ned_pos);
    entity->pos.geo = ecef_to_geo(entity->pos.ecef);
}

static void set_entity_heading(EntityType* entity, float new_heading)
{
    // TODO(scott): flesh this out when we have more coordinate systems, etc.
    entity->aircraft.heading = new_heading;
}

inline static uint32_t get_entity_index(SimState* state, EntityType* entity)
{
    for (uint32_t entity_index = 0; entity_index < state->num_entities; entity_index++)
    {
        if (state->entities + entity_index == entity)
        {
            return entity_index;
        }
    }
    return -1;
}

static void add_to_shoot_list(SimState* state, uint32_t entity_index)
{
    assert(ArrayCount(state->shoot_list) > state->num_shoot_list + 1);
    state->shoot_list[state->num_shoot_list++] = entity_index;
}

static void add_to_shoot_list(SimState* state, EntityType* entity)
{
    uint32_t entity_index = get_entity_index(state, entity);
    if (entity_index >= 0)
    {
        add_to_shoot_list(state, entity_index);
        return;
    }
    assert(!"Entity wasn't in the entities list!");
}

static void remove_from_shoot_list(SimState* state, uint32_t entity_index)
{
    // order needs to be maintained when one is removed, therefore we can't
    // just copy the last item in the list
    for (uint32_t index = 0; index < state->num_shoot_list; index++)
    {
        if (state->shoot_list[index] == entity_index)
        {
            memcpy(state->shoot_list + index, state->shoot_list + index + 1, sizeof(uint32_t) * (state->num_shoot_list - index));
            state->num_shoot_list--;
            return;
        }
    }
    assert(!"tried to remove an entity that isn't in the shoot list!");
}

static void remove_from_shoot_list(SimState* state, EntityType* entity)
{
    uint32_t entity_index = get_entity_index(state, entity);
    if (entity_index >= 0)
    {
        remove_from_shoot_list(state, entity_index);
        return;
    }
    assert(!"tried to remove an entity that isn't in the shoot list!");
}

static int get_shoot_list_priority(SimState* state, uint32_t entity_index)
{
    for (uint32_t index = 0; index < state->num_shoot_list; index++)
    {
        if (state->shoot_list[index] == entity_index)
        {
            return index;
        }
    }
    return -1;
}

static int get_shoot_list_priority(SimState* state, EntityType* entity)
{
    uint32_t entity_index = get_entity_index(state, entity);
    if (entity_index >= 0)
    {
        return get_shoot_list_priority(state, entity_index);
    }
    return -1;
}

static bool target_in_shoot_list(SimState* state, uint32_t entity_index)
{
    return get_shoot_list_priority(state, entity_index) > -1;
}
