
// ****************************************************************************
// Explanation of coordinate systems
//
// Geodetic: 
//    Latitude - angle north/south of the equator
//    Longitude - angle east/west of the prime meridian
//    Altitude - height above Earth ellipsoid (WGS-84)
//
// Earth-Centered, Earth-Fixed (ECEF):
//    X - passes through the equator at the prime meridian
//    Y - passes through the equator 90 degrees east of the prime meridian
//    Z - passes through the north pole
// 
// North-East-Down (NED):
//    N - points to north in the local-level plane of the origin (usually ownship)
//    E - points to east in the local-level plane of the origin (usually ownship)
//    D - points towards the geodetic center of the earth from the origin (usually ownship)


static Vec3 geo_to_ecef(Vec3 geo_pos)
{
    Vec3 result;
    float clat = cosf(RADIANS(geo_pos.lat));
    float slat = sinf(RADIANS(geo_pos.lat));
    float clon = cosf(RADIANS(geo_pos.lon));
    float slon = sinf(RADIANS(geo_pos.lon));
    float N = WGS84_A / sqrtf(1.0f - WGS84_E2 * slat * slat);
    result.x = (N + geo_pos.alt) * clat * clon;
    result.y = (N + geo_pos.lat) * clat * slon;
    result.z = (N * (1.0f - WGS84_E2) + geo_pos.alt) * slat;
    return result;
}

static Vec3 ecef_to_ned(Vec3 geo_origin, Vec3 ecef_pos)
{
    Vec3 result;
    float clat = cosf(RADIANS(geo_origin.lat));
    float slat = sinf(RADIANS(geo_origin.lat));
    float clon = cosf(RADIANS(geo_origin.lon));
    float slon = sinf(RADIANS(geo_origin.lon));
    Vec3 ecef_orig = geo_to_ecef(geo_origin);
    float dx = ecef_pos.x - ecef_orig.x;
    float dy = ecef_pos.y - ecef_orig.y;
    float dz = ecef_pos.z - ecef_orig.z;
    result.n = -slat*clon*dx - slat*slon*dy + clat*dz;
    result.e = -slon*dx + clon*dy;
    result.d = -(clat*clon*dx + clat*slon*dy + slat*dz);
    return result;
}

static Vec3 ned_to_ecef(Vec3 geo_origin, Vec3 ned_pos)
{
    Vec3 result;
    float clat = cosf(RADIANS(geo_origin.lat));
    float slat = sinf(RADIANS(geo_origin.lat));
    float clon = cosf(RADIANS(geo_origin.lon));
    float slon = sinf(RADIANS(geo_origin.lon));
    Vec3 ecef_origin = geo_to_ecef(geo_origin);
    result.x = -ned_pos.e*slon - ned_pos.n*clon*slat - ned_pos.d*clon*clat;
    result.y = ned_pos.e*clon - ned_pos.n*slon*slat - ned_pos.d*slon*clat;
    result.z = ned_pos.n*clat - ned_pos.d*slat;
    result += ecef_origin;
    return result;
}

#define _wgs_a1 (WGS84_A * WGS84_E2)
#define _wgs_a2 (_wgs_a1 * _wgs_a1)
#define _wgs_a3 (_wgs_a1 * WGS84_E2 * 0.5f)
#define _wgs_a4 (2.5f * _wgs_a2)
#define _wgs_a5 (_wgs_a1 + _wgs_a3)
#define _wgs_a6 (1.0f - WGS84_E2)

static Vec3 ecef_to_geo(Vec3 ecef_pos)
{
    // ref: http://danceswithcode.net/engineeringnotes/geodetic_to_ecef/geodetic_to_ecef.html
    Vec3 result;
    float zp = ABS(ecef_pos.z);
    float w2 = ecef_pos.x*ecef_pos.x + ecef_pos.y*ecef_pos.y;
    float w = sqrtf(w2);
    float r2 = w2 + ecef_pos.z*ecef_pos.z;
    float r = sqrtf(r2);
    float s2 = ecef_pos.z*ecef_pos.z / r2;
    float c2 = w2 / r2;
    float u = _wgs_a2 / r;
    float v = _wgs_a3 - _wgs_a4 / r;
    float c;
    float s;
    float ss;
    if (c2 > 0.3f)
    {
        s = (zp / r)*(1.0f + c2*(_wgs_a1 + u + s2*v) / r);
        result.lat = asinf(s);
        ss = s*s;
        c = sqrtf(1.0f - ss);
    }
    else
    {
        c = (w / r)*(1.0f - s2*(_wgs_a5 - u - c2*v) / r);
        result.lat = acosf(c);
        ss = 1.0f - c*c;
        s = sqrtf(ss);
    }
    float g = 1.0f - WGS84_E2*ss;
    float rg = WGS84_A / sqrtf(g);
    float rf = _wgs_a6 * rg;
    u = w - rg*c;
    v = zp - rf*s;
    float f = c*u + s*v;
    float m = c*v - s*u;
    float p = m / (rf / g + f);
    result.lon = atan2f(ecef_pos.y, ecef_pos.x);
    result.lat += p;
    result.alt = f + m*p / 2.0f;
    if (ecef_pos.z < 0)
    {
        result.lat *= -1.0f;
    }
    return result;
}

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
