#define PI 3.14159265359f
#define TWO_PI 2.0f*PI
#define PI_OVER_TWO PI*0.5f

#define RADIANS(d) ((d) * PI / 180)
#define DEGREES(r) ((r) * 180 / PI)

typedef union Vec2 {
    struct {
        float x, y;
    };
    float e[2];
} Vec2;


typedef union Vec3 {
    struct {
        float x, y, z;
    };
    struct {
        float r, g, b;
    };
    float e[3];
} Vec3;