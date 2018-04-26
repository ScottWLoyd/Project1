
//
// Vec3 Operations
// 

inline Vec2 vec2(float x, float y)
{
    Vec2 result;
    result.x = x;
    result.y = y;
    return result;
}

inline Vec2 operator+(Vec2 left, Vec2 right)
{
    Vec2 result;
    result.x = left.x + right.x;
    result.y = left.y + right.y;
    return result;
}

inline Vec2 operator-(Vec2 left, Vec2 right)
{
    Vec2 result;
    result.x = left.x - right.x;
    result.y = left.y - right.y;
    return result;
}

inline Vec2 operator*(Vec2 left, Vec2 right)
{
    Vec2 result;
    result.x = left.x * right.x;
    result.y = left.y * right.y;
    return result;
}

inline Vec2 operator*(float scale, Vec2 right)
{
    Vec2 result;
    result.x = scale * right.x;
    result.y = scale * right.y;
    return result;
}

inline Vec2& operator*=(Vec2& left, float scale)
{
    left = scale * left;
    return left;
}

inline Vec2& operator+=(Vec2& left, Vec2& right)
{
    left = left + right;
    return left;
}

inline Vec2& operator-=(Vec2& left, Vec2& right)
{
    left = left - right;
    return left;
}

inline Vec2& operator*=(Vec2& left, Vec2& right)
{
    left = left * right;
    return left;
}

float mag2(Vec2 v)
{
    float result;
    result = v.x*v.x + v.y*v.y;
    return result;
}

float mag(Vec2 v)
{
    float result;
    result = sqrtf(mag2(v));
    return result;
}

//
// Vec3 Operations
// 

static inline Vec3 vec3(float x, float y, float z)
{
    Vec3 result;
    result.x = x;
    result.y = y;
    result.z = z;
    return result;
}

inline Vec3 operator+(Vec3 left, Vec3 right)
{
    Vec3 result;
    result.x = left.x + right.x;
    result.y = left.y + right.y;
    result.z = left.z + right.z;
    return result;
}

inline Vec3 operator-(Vec3 left, Vec3 right)
{
    Vec3 result;
    result.x = left.x - right.x;
    result.y = left.y - right.y;
    result.z = left.z - right.z;
    return result;
}

inline Vec3 operator*(Vec3 left, Vec3 right)
{
    Vec3 result;
    result.x = left.x * right.x;
    result.y = left.y * right.y;
    result.z = left.z * right.z;
    return result;
}

inline Vec3 operator*(float scale, Vec3 right)
{
    Vec3 result;
    result.x = scale * right.x;
    result.y = scale * right.y;
    result.z = scale * right.z;
    return result;
}

inline Vec3& operator*=(Vec3& left, float scale)
{
    left = scale * left;
    return left;
}

inline Vec3& operator+=(Vec3& left, Vec3& right)
{
    left = left + right;
    return left;
}

inline Vec3& operator-=(Vec3& left, Vec3& right)
{
    left = left - right;
    return left;
}

inline Vec3& operator*=(Vec3& left, Vec3& right)
{
    left = left * right;
    return left;
}

float mag2(Vec3 v)
{
    float result;
    result = v.x*v.x + v.y*v.y + v.z*v.z;
    return result;
}

float mag(Vec3 v)
{
    float result;
    result = sqrtf(mag2(v));
    return result;
}

//
// Geometry
//

static bool box_contains_point(Box box, Vec2 point)
{
    return point.x >= box.top_left.x && point.x < box.bottom_right.x &&
        point.y >= box.top_left.y && point.y < box.bottom_right.y;
}

static bool circle_contains_point(Circle circle, Vec2 point)
{
    float difference = mag(point - circle.center);
    return difference <= circle.radius;
}