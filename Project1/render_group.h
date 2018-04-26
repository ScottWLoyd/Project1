

enum RenderObjectType {
    RenderObjectTexturedRect,
    RenderObjectCircle,
    RenderObjectFillCircle,
    RenderObjectLine,
    RenderObjectText,
    RenderObjectCompass,
};

struct RenderObject {
    RenderObjectType type;
    Vec3 color;
    // NOTE(scott): these coordinates are in screen pixels relative to
    // the center of the screen!
    union {
        struct {
            GLuint texture_id;
            Vec2 center;
            Vec2 dim;
            float rotation;
            Vec2 scale;
        } textured_rect;
        struct {
            Vec2 center;
            float radius;
            GLfloat line_width;
        } circle;
        struct {
            Vec2 center;
            float radius;
            float rotation;
        } compass;
        struct {
            Vec2 start;
            Vec2 end;
            GLfloat line_width;
        } line;
        struct {
            char text[20];
            float size;
            float x;
            float y;
        } text;
    };
};

enum BoundingGeometryType {
    BoundingGeometry_Circle,
    BoundingGeometry_Box,
};

struct RenderGroup {
    size_t max_render_objects;
    size_t num_render_objects;
    RenderObject* render_objects;

    bool pickable;
    struct {
        BoundingGeometryType type;
        // NOTE(scott): these coordinates are in screen pixels relative to
        // the center of the screen!
        union {
            Circle circle;
            Box box;
        };
    } bounding;

    uint32_t entity_index;
};
