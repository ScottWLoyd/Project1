

static void set_entity_pos(EntityType* entity, Vec3 new_pos)
{
    // TODO(scott): flesh this out when we have more coordinate systems, etc.
    entity->pos = new_pos;
}

static void set_entity_heading(EntityType* entity, float new_heading)
{
    // TODO(scott): flesh this out when we have more coordinate systems, etc.
    entity->aircraft.heading = new_heading;
}

