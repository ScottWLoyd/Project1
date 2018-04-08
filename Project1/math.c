#define PI 3.14159265359f
#define TWO_PI 2.0f*PI
#define PI_OVER_TWO PI*0.5f

typedef union Vec2 {
    struct {
        float x, y;
    };
    float e[2];
} Vec2;

static inline Vec2 vec2(float x, float y)
{
    Vec2 result;
    result.x = x;
    result.y = y;
    return result;
}

typedef union Vec3 {
    struct {
        float x, y, z;
    };
    struct {
        float r, g, b;
    };
    float e[3];
} Vec3;