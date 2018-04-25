#define PI 3.14159265359f
#define TWO_PI 2.0f*PI
#define PI_OVER_TWO PI*0.5f

// WGS-84 Constants
#define WGS84_A     20925646.0f         // semi-major axis (feet)
#define WGS84_1_f   298.257223563f      // inverse flattening
#define WGS84_E     8.1819190842622e-2f // first eccentricity
#define WGS84_E2    6.69437999014e-3f   // first eccentricity squared

#define RADIANS(d) ((d) * PI / 180)
#define DEGREES(r) ((r) * 180 / PI)

#define FT_TO_NM(ft) ((ft)*0.0001645788f)
#define NM_TO_FT(nm) ((nm)*6076.118f)

#define ABS(val) ((val<0)?-val:val)
#define MAX(x,y) (x > y ? x : y)
#define MIN(x,y) (x < y ? x : y)
#define CLAMP(min, val, max) ((val)<(min)?(val)=(min):(val)>(max)?(val)=(max):(void)(val))
#define ArrayCount(arr) (sizeof(arr)/sizeof(arr[0]))

union Vec2 {
    struct {
        float x, y;
    };
    float e[2];
};

Vec2 operator+(Vec2 left, Vec2 right);
Vec2 operator-(Vec2 left, Vec2 right);
Vec2 operator*(Vec2 left, Vec2 right);
Vec2 operator*(float scale, Vec2 right);
Vec2& operator*=(Vec2& left, float scale);
Vec2& operator+=(Vec2& left, Vec2& right);
Vec2& operator-=(Vec2& left, Vec2& right);
Vec2& operator*=(Vec2& left, Vec2& right);

float mag2(Vec2 v);
float mag(Vec2 v);

union Vec3 {
    struct {
        float x, y, z;
    };
    struct {
        float r, g, b;
    };
    struct {
        float lat, lon, alt;
    };
    struct {
        float n, e, d;
    };
    float elem[3];
};

Vec3 operator+(Vec3 left, Vec3 right);
Vec3 operator-(Vec3 left, Vec3 right);
Vec3 operator*(Vec3 left, Vec3 right);
Vec3 operator*(float scale, Vec3 right);
Vec3& operator*=(Vec3& left, float scale);
Vec3& operator+=(Vec3& left, Vec3& right);
Vec3& operator-=(Vec3& left, Vec3& right);
Vec3& operator*=(Vec3& left, Vec3& right);

float mag2(Vec3 v);
float mag(Vec3 v);