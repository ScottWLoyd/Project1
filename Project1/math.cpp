
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
    result.col[0].x = m.elem[0];
    result.col[1].x = m.elem[1];
    result.col[0].y = m.elem[2];
    result.col[1].y = m.elem[3];
    return result;
}

Mat2 inverse(Mat2 m)
{
    Mat2 result;
    float d = det(m);
    assert(d != 0);
    float ood = 1.0f / d;
    result.col[0].x = +m.col[1].y * ood;
    result.col[0].y = -m.col[0].y * ood;
    result.col[1].x = -m.col[1].x * ood;
    result.col[1].y = +m.col[0].x * ood;
    return result;
}

Mat2 operator*(Mat2 left, Mat2 right)
{
    Mat2 result;
    result.col[0].x = left.col[0].x * right.col[0].x + left.col[1].x * right.col[0].y;
    result.col[0].y = left.col[0].y * right.col[0].x + left.col[1].y * right.col[0].y;
    result.col[1].x = left.col[0].x * right.col[1].x + left.col[1].x * right.col[1].y;
    result.col[1].y = left.col[0].y * right.col[1].x + left.col[1].y * right.col[1].y;
    return result;
}

Mat2 operator*(float scalar, Mat2 m)
{
    Mat2 result = m;
    for (int i = 0; i < ArrayCount(result.elem); i++)
    {
        result.elem[i] *= scalar;
    }
    return result;
}

float det(Mat3 m)
{
    float result = 0;
    result += m.col[0].x*m.col[1].y*m.col[2].z;
    result += m.col[1].x*m.col[2].y*m.col[0].z;
    result += m.col[2].x*m.col[0].y*m.col[1].z;
    result -= m.col[0].z*m.col[1].y*m.col[2].x;
    result -= m.col[1].z*m.col[2].y*m.col[0].x;
    result -= m.col[2].z*m.col[0].y*m.col[1].x;
    return result;
}

Mat3 transpose(Mat3 m)
{
    Mat3 result;
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            result.col[i].elem[j] = m.col[j].elem[i];
        }
    }
    return result;
}

Mat3 inverse(Mat3 m)
{
    Mat3 result;
    float d = det(m);
    assert(d != 0);
    float ood = 1.0f / d;

    result.col[0].x =  (m.col[1].y * m.col[2].z - m.col[2].y * m.col[1].z) * ood;
    result.col[1].x = -(m.col[1].x * m.col[2].z - m.col[2].x * m.col[1].z) * ood;
    result.col[2].x =  (m.col[1].x * m.col[2].y - m.col[2].x * m.col[1].y) * ood;
    result.col[0].y = -(m.col[0].y * m.col[2].z - m.col[2].y * m.col[0].z) * ood;
    result.col[1].y =  (m.col[0].x * m.col[2].z - m.col[2].x * m.col[0].z) * ood;
    result.col[2].y = -(m.col[0].x * m.col[2].y - m.col[2].x * m.col[0].y) * ood;
    result.col[0].z =  (m.col[0].y * m.col[1].z - m.col[1].y * m.col[0].z) * ood;
    result.col[1].z = -(m.col[0].x * m.col[1].z - m.col[1].x * m.col[0].z) * ood;
    result.col[2].z =  (m.col[0].x * m.col[1].y - m.col[1].x * m.col[0].y) * ood;

    return result;
}

Mat3 operator*(Mat3 left, Mat3 right)
{
    Mat3 result;
    result.col[0].x = left.col[0].x * right.col[0].x + left.col[1].x * right.col[0].y + left.col[2].x * right.col[0].z;
    result.col[0].y = left.col[0].y * right.col[0].x + left.col[1].y * right.col[0].y + left.col[2].y * right.col[0].z;
    result.col[0].z = left.col[0].z * right.col[0].x + left.col[1].z * right.col[0].y + left.col[2].z * right.col[0].z;
    result.col[1].x = left.col[0].x * right.col[1].x + left.col[1].x * right.col[1].y + left.col[2].x * right.col[1].z;
    result.col[1].y = left.col[0].y * right.col[1].x + left.col[1].y * right.col[1].y + left.col[2].y * right.col[1].z;
    result.col[1].z = left.col[0].z * right.col[1].x + left.col[1].z * right.col[1].y + left.col[2].z * right.col[1].z;
    result.col[2].x = left.col[0].x * right.col[2].x + left.col[1].x * right.col[2].y + left.col[2].x * right.col[2].z;
    result.col[2].y = left.col[0].y * right.col[2].x + left.col[1].y * right.col[2].y + left.col[2].y * right.col[2].z;
    result.col[2].z = left.col[0].z * right.col[2].x + left.col[1].z * right.col[2].y + left.col[2].z * right.col[2].z;
    return result;
}

Mat3 operator*(float scalar, Mat3 m)
{
    Mat3 result = m;
    for (int i = 0; i < ArrayCount(result.elem); i++)
    {
        result.elem[i] *= scalar;
    }
    return result;
}

Mat4 transpose(Mat4 m)
{
    Mat4 result;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            result.col[i].elem[j] = m.col[j].elem[i];
        }
    }
    return result;
}

Mat4 operator*(Mat4 left, Mat4 right)
{
    Mat4 result;
    for (int r = 0; r < 4; r++) // NOTE(scott): rows (of left)
    {
        for (int c = 0; c < 4; c++) // NOTE(scott): columns (of right)
        {
            for (int i = 0; i < 4; i++) // NOTE(scott): columns of left, rows of right
            {
                result.col[c].elem[r] += left.col[i].elem[r] * right.col[c].elem[i];
            }
        }
    }
    return result;
}

Mat4 operator*(float scalar, Mat4 m)
{
    Mat4 result;
    for (int i = 0; i < ArrayCount(m.elem); i++)
    {
        result.elem[i] = scalar * m.elem[i];
    }
    return result;
}

void matrix_test(void)
{
    // 2x2
    Mat2 mat1 = { 1, 3, 2, 4 };
    float det1 = det(mat1);
    assert(det1 == (1 * 4 - 2 * 3));
    Mat2 t1 = transpose(mat1);
    assert(t1.col[0].x == 1); assert(t1.col[1].x == 3); assert(t1.col[0].y == 2); assert(t1.col[1].y == 4);
    Mat2 i1 = inverse(mat1);
    assert(i1.col[0].x == -2); assert(i1.col[1].x == 1); assert(i1.col[0].y == 1.5f); assert(i1.col[1].y == -0.5f);
    Mat2 mat2 = { 0, 0, 1, 0 };
    Mat2 prod1 = mat1 * mat2;
    assert(prod1.col[0].x == 0); assert(prod1.col[0].y == 0); assert(prod1.col[1].x == 1); assert(prod1.col[1].y == 3);

    // 3x3
    Mat3 mat3;
    for (int i = 0; i < ArrayCount(mat3.elem); i++)
    {
        mat3.elem[i] = (float)i;
    }
    float det3 = det(mat3);
    assert(det3 == 0);
    mat3.col[0].x = 1;
    assert(det(mat3) == -3);
    Mat3 mat4 = { 1, 0, 5, 2, 1, 6, 3, 4, 0 };
    assert(det(mat4) == 1);
    Mat3 i3 = inverse(mat4);
    assert(i3.elem[0] == -24); assert(i3.elem[1] == 20); assert(i3.elem[2] == -5);
    assert(i3.elem[3] == 18); assert(i3.elem[4] == -15); assert(i3.elem[5] == 4);
    assert(i3.elem[6] == 5); assert(i3.elem[7] == -4); assert(i3.elem[8] == 1);
    Mat3 prod2 = mat3 * mat4;
    assert(prod2.elem[0] == 31); assert(prod2.elem[1] == 36); assert(prod2.elem[2] == 42);
    assert(prod2.elem[3] == 41); assert(prod2.elem[4] == 48); assert(prod2.elem[5] == 57);
    assert(prod2.elem[6] == 15); assert(prod2.elem[7] == 19); assert(prod2.elem[8] == 26);

    // 4x4
    Mat4 m;
    for (int i = 0; i < ArrayCount(m.elem); i++)
    {
        m.elem[i] = (float)i;
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