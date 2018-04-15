


static inline Vec2 vec2(float x, float y)
{
    Vec2 result;
    result.x = x;
    result.y = y;
    return result;
}

static inline Vec3 vec3(float x, float y, float z)
{
    Vec3 result;
    result.x = x;
    result.y = y;
    result.z = z;
    return result;
}

static inline Vec3 vec3_add(Vec3 a, Vec3 b)
{
    Vec3 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    return result;
}

static inline Vec3 vec3_sub(Vec3 a, Vec3 b)
{
    Vec3 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    return result;
}