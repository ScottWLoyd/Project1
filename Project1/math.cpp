
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
// Matrices
//

float det(Mat2 m)
{
    float result;
    result = m.col[0].x * m.col[1].y - m.col[1].x * m.col[0].y;
    return result;
}

Mat2 transpose(Mat2 m)
{
    Mat2 result;
    result.col[0].x = m.e[0];
    result.col[1].x = m.e[1];
    result.col[0].y = m.e[2];
    result.col[1].y = m.e[3];
    return result;
}

Mat2 inverse(Mat2 m)
{
    Mat2 result;
    float d = det(m);
    result.col[0].x = +m.col[1].y / d;
    result.col[0].y = -m.col[0].y / d;
    result.col[1].x = -m.col[1].x / d;
    result.col[1].y = +m.col[0].x / d;
    return result;
}

Mat4 transpose(Mat4 m)
{
    Mat4 result;
    result.col[0].x = m.col[0].x; result.col[0].y = m.col[1].x; result.col[0].z = m.col[2].x; result.col[0].w = m.col[3].x;
    result.col[1].x = m.col[0].y; result.col[1].y = m.col[1].y; result.col[1].z = m.col[2].y; result.col[1].w = m.col[3].y;
    result.col[2].x = m.col[0].z; result.col[2].y = m.col[1].z; result.col[2].z = m.col[2].z; result.col[2].w = m.col[3].z;
    result.col[3].x = m.col[0].w; result.col[3].y = m.col[1].w; result.col[3].z = m.col[2].w; result.col[3].w = m.col[3].w;
    return result;
}

Mat4 inverse(Mat4 m)
{
    Mat4 result = { 0 };
    assert(0);
    return result;
}

void matrix_test(void)
{
    Mat2 mat1 = { 1, 3, 2, 4 };
    float det1 = det(mat1);
    assert(det1 == (1 * 4 - 2 * 3));
    Mat2 t1 = transpose(mat1);
    assert(t1.col[0].x == 1); assert(t1.col[1].x == 3); assert(t1.col[0].y == 2); assert(t1.col[1].y == 4);
    Mat2 i1 = inverse(mat1);
    assert(i1.col[0].x == -2); assert(i1.col[1].x == 1); assert(i1.col[0].y == 1.5f); assert(i1.col[1].y == -0.5f);
    

    Mat4 m;
    for (int i = 0; i < 16; i++)
    {
        m.e[i] = (float)i;
    }
    Mat4 t = transpose(m);
    assert(t.col[0].x == 0); assert(t.col[1].x == 1); assert(t.col[2].x == 2); assert(t.col[3].x == 3);
    assert(t.col[0].y == 4); assert(t.col[1].y == 5); assert(t.col[2].y == 6); assert(t.col[3].y == 7);
    assert(t.col[0].z == 8); assert(t.col[1].z == 9); assert(t.col[2].z == 10); assert(t.col[3].z == 11);
    assert(t.col[0].w == 12); assert(t.col[1].w == 13); assert(t.col[2].w == 14); assert(t.col[3].w == 15);
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