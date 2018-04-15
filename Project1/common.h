
typedef enum AircraftKind {
    AircraftKind_F15,
    AircraftKind_F22,
    AircraftKind_SU35,
} AircraftKind;

typedef enum IffStatusType {
    IffStatusType_None,
    IffStatusType_Friendly,
    IffStatusType_Neutral,
    IffStatusType_Hostile,

    IffStatusType_Count,
} IffStatusType;

typedef enum EntityKind {
    EntityKind_None,
    EntityKind_Ownship,
    EntityKind_Aircraft,
    EntityKind_Symbol,
} EntityKind;

typedef struct EntityType {
    EntityKind kind;
    Vec3 pos;
    IffStatusType iff_status;
    union {
        struct {
            AircraftKind kind;
            float heading;
        } aircraft;
    };
} EntityType;

typedef struct SimState {
    bool initialized;

    float dt;

    MemoryArena sim_arena;
    EntityType* entities[1024];
    uint32_t num_entities;

    uint32_t ownship_index;

    RenderState render_state;
} SimState;