
enum AircraftKind {
    AircraftKind_F15,
    AircraftKind_F22,
    AircraftKind_SU35,
};

enum IffStatusType {
    IffStatusType_None,
    IffStatusType_Friendly,
    IffStatusType_Neutral,
    IffStatusType_Hostile,

    IffStatusType_Count,
};

enum EntityKind {
    EntityKind_None,
    EntityKind_Ownship,
    EntityKind_Aircraft,
    EntityKind_Symbol,
};

struct EntityType {
    EntityKind kind;
    IffStatusType iff_status;
    Vec3 geo_pos;   // lat, lon, alt; degrees, feet
    Vec3 ecef_pos;  // feet
    Vec3 ned_pos;   // north, east, down; feet
    union {
        struct {
            AircraftKind kind;
            float heading;
        } aircraft;
    };
};

struct SimState {
    bool initialized;

    float dt;

    // These are set to true for the frame in which they cross a 
    // periodic threshold. They are then set to false at the end
    // of the frame.
    struct {
        bool one_hertz;
        bool five_hertz;
        bool ten_hertz;
        bool twenty_hertz;
    } periodic;

    MemoryArena sim_arena;
    EntityType* entities[1024];
    uint32_t num_entities;

    uint32_t ownship_index;
    uint32_t shoot_list[30];
    uint32_t num_shoot_list;

    RenderState render_state;
};


int get_shoot_list_priority(SimState* state, EntityType* entity);