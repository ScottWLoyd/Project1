
// NOTE(scott): changes to this list need to be reflected in the 
// imgui_impl file for combobox selection
enum AircraftKind {
    AircraftKind_F15,
    AircraftKind_F22,
    AircraftKind_SU35,
};

// NOTE(scott): changes to this list need to be reflected in the 
// imgui_impl file for combobox selection
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
    struct {
        Vec3 geo;   // lat, lon, alt; degrees, feet
        Vec3 ecef;  // feet
        Vec3 ned;   // north, east, down; feet
    } pos;
    struct {
        Vec3 ecef;  // feet / sec
        Vec3 ned;   // north, east, down; feet / sec
    } vel;
    struct {
        Vec3 ecef;  // feet / sec
        Vec3 ned;   // north, east, down; feet / sec
    } acc;
    union {
        struct {
            AircraftKind kind;
            float heading;  // north referenced; degrees
        } aircraft;
    };
};

struct SimState {
    bool initialized;

    struct {
        float dt;
        float timescale;
        float effective_elapsed;
    } time;

    // These are set to true for the frame in which they cross a 
    // periodic threshold. They are then set to false at the end
    // of the frame.
    struct {
        bool one_hertz;
        bool five_hertz;
        bool ten_hertz;
        bool twenty_hertz;
    } periodic;

    MemoryArena* sim_arena;
    EntityType entities[1024];
    uint32_t num_entities;

    uint32_t ownship_index;
    uint32_t shoot_list[30];
    uint32_t num_shoot_list;

    RenderState render_state;
};

void set_entity_ned_pos(EntityType* entity, Vec3 new_ned_pos, Vec3 ownship_geo_pos); 
void set_entity_heading(EntityType* entity, float new_heading);
int get_shoot_list_priority(SimState* state, EntityType* entity);
uint32_t get_entity_index(SimState* state, EntityType* entity);