
typedef enum EntityKind {
    EntityKind_None,
    EntityKind_Ownship,
    EntityKind_Aircraft,
} EntityKind;

typedef struct EntityType {
    EntityKind kind;
    Vec3 pos;
} EntityType;

typedef struct SimState {        
    bool initialized;

    float dt;

    MemoryArena sim_arena;
    EntityType* entities[1024];
    uint32_t num_entities;

    RenderState render_state;
} SimState;