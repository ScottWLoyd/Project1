
//
// Constants
//

#define PI 3.14159265359f
#define TWO_PI 2.0f*PI
#define PI_OVER_TWO PI*0.5f

#define RADIANS(d) ((d) * PI / 180)
#define DEGREES(r) ((r) * 180 / PI)

#define FT_TO_NM(ft) ((ft)*0.0001645788f)
#define NM_TO_FT(nm) ((nm)*6076.118f)

#define KNTS_TO_FPS(knts) ((knts)*1.68781f)
#define FPS_TO_KNTS(fps) ((fps)*0.592484f))

#define ABS(val) ((val<0)?-val:val)
#define MAX(x,y) (x > y ? x : y)
#define MIN(x,y) (x < y ? x : y)
#define CLAMP(min, val, max) ((val)<(min)?(val)=(min):(val)>(max)?(val)=(max):(void)(val))
#define ArrayCount(arr) (sizeof(arr)/sizeof(arr[0]))

//
// Vectors
//

union Vec2 {
    struct {
        float x, y;
    };
    float elem[2];
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
        float u, v, w;
    };
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
    struct {
        float az, el, range;
    };
    struct {
        float yaw, pitch, roll;
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

union Vec4 {
    struct {
        float x, y, z, w;
    };
    struct {
        float r, g, b, a;
    };
    struct {
        Vec3 xyz;
        float w;
    };
    float elem[4];
};

//
// Matracies
//

union Mat2 {
    struct {
        Vec2 x, y;
    };
    Vec2 col[2];
    float elem[4];
};

float det(Mat2);
Mat2 transpose(Mat2);
Mat2 inverse(Mat2);
Mat2 operator*(Mat2 left, Mat2 right);
Mat2 operator*(float scalar, Mat2 m);

union Mat3 {
    struct {
        Vec3 x, y, z;
    };
    Vec3 col[3];
    float elem[9];
};

float det(Mat3);
Mat3 transpose(Mat3);
Mat3 inverse(Mat3);
Mat3 operator*(Mat3 left, Mat3 right);
Mat3 operator*(float scalar, Mat3 m);

union Mat4 {
    struct {
        Vec4 x, y, z, w;
    };
    Vec4 col[4];
    float elem[16];
};

Mat4 transpose(Mat4);
Mat4 operator*(Mat4 left, Mat4 right);
Mat4 operator*(float scalar, Mat4 m);

//
// Geometry
// 

struct Box {
    Vec2 top_left;
    Vec2 bottom_right;
};

struct Circle {
    Vec2 center;
    float radius;
};