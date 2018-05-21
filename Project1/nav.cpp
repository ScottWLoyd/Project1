// ****************************************************************************
// Explanation of coordinate systems
//
// TODO(scott): delete the coordinate systems here we don't care about
// TODO(scott): according to the 2005 EGI ICD, the Hayford ellipsoid is used instead
// of the WGS-84.  Need to investigate. 
//
// Wander Angle - positive (right-hand) rotation of the Navigational frame about 
//    the up-axis from east and north to the navigational X & Y
//
// Body:
//    U - positive forward along aircraft longitudinal axis
//    V - positive towards aircraft right wing
//    W - positive down
// 
// Earth-Centered, Earth-Fixed (ECEF):
//    X - passes through the equator at the prime meridian
//    Y - passes through the equator 90 degrees east of the prime meridian
//    Z - passes through the north pole
// 
// Geographic: 
//    Latitude - angle north/south of the equator
//    Longitude - angle east/west of the prime meridian
//    Altitude - height above Earth ellipsoid (WGS-84?)
//
// Navigational:
//    X - horizontal plane rotated from east by the wander angle
//    Y - horizontal plane rotated from north by the wander angle
//    Z - local up
//
// Geodetic North-East-Down (NED):
//    N - points to north in the local-level plane 
//    E - points to east in the local-level plane 
//    D - points towards the geodetic center of the earth 

#ifndef _WGS84
#define _WGS84
// WGS-84 Constants
#define WGS84_A     20925646.0f         // semi-major axis (feet)
#define WGS84_1_f   298.257223563f      // inverse flattening
#define WGS84_E     8.1819190842622e-2f // first eccentricity
#define WGS84_E2    6.69437999014e-3f   // first eccentricity squared
#endif


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
#undef _wgs_a1 
#undef _wgs_a2 
#undef _wgs_a3 
#undef _wgs_a4 
#undef _wgs_a5 
#undef _wgs_a6 

static Vec3 geo_to_ecef(Vec3 geo_pos)
{
    Vec3 result;
    float clat = cosf(RADIANS(geo_pos.lat));
    float slat = sinf(RADIANS(geo_pos.lat));
    float clon = cosf(RADIANS(geo_pos.lon));
    float slon = sinf(RADIANS(geo_pos.lon));
    float N = WGS84_A / sqrtf(1.0f - WGS84_E2 * slat * slat);
    result.x = (N + geo_pos.alt) * clat * clon;
    result.y = (N + geo_pos.alt) * clat * slon;
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

// TODO(scott): test this!!!
static Vec3 sphere_to_ned(Vec3 sphere_pos)
{
    Vec3 ned_pos;
    float azRadians = RADIANS(sphere_pos.az); // wrt to True North
    float elRadians = RADIANS(sphere_pos.el);
    ned_pos.e = sphere_pos.range * sinf(azRadians) * cosf(elRadians);
    ned_pos.n = sphere_pos.range * cosf(azRadians) * cosf(elRadians);
    ned_pos.d = -sphere_pos.range * sinf(elRadians);
}

// TODO(scott): test this!!!
static Vec3 sphere_to_geo(Vec3 shooter_geo, float shooter_heading, Vec3 target_sphere)
{
    target_sphere.az += shooter_heading;
    Vec3 target_ned = sphere_to_ned(target_sphere);
    Vec3 target_ecef = ned_to_ecef(shooter_geo, target_ned);
    Vec3 target_geo = ecef_to_geo(target_geo);
    return target_geo;
}

static Vec3 geo_to_body(Vec3 shooter_geo, float shooter_heading, Vec3 target_geo)
{
    // TODO(scott): write me
}