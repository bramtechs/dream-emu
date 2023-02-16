// contains more "advanced" components and entities, not needed for some types of games
// can be removed from engine
// TODO: Make optional with build flag

enum PlayerPose {
    POSE_IDLE,
    POSE_WALK,
    POSE_SLIDE,
    POSE_JUMP,
    POSE_FALL,
    POSE_DUCK,
};
constexpr char* PlayerPoseNames[] = {
    STRING(POSE_IDLE),
    STRING(POSE_WALK),
    STRING(POSE_SLIDE),
    STRING(POSE_JUMP),
    STRING(POSE_FALL),
    STRING(POSE_DUCK),
};

enum PlaybackMode {
    PLAY_LOOP,
    PLAY_PING_PONG,
    PLAY_ONCE,
};

constexpr int COMP_PHYS_BODY   = 10;
constexpr int COMP_ANIM_PLAYER = 11;
constexpr int COMP_PLAT_PLAYER = 12;

struct SheetAnimation {
    std::string name; // description 
    std::string sheetName; // name of texture
    Vector2 origin; // top left of sector
    Vector2 cellSize; // size of each cell
    uint count; // number of cells in anim
    PlaybackMode mode;
    float fps;
};

struct PhysicsBody {
    bool initialized;
    bool dynamic;

    float density;
    float friction;

    b2Body* body;

    PhysicsBody(float density=30.f, float friction=0.5f);
    PhysicsBody(bool isDynamic);
    ~PhysicsBody(); // TODO: dispose
    
    Vector2 position();
    float angle();
};

struct PlatformerPlayer {
    float moveSpeed;
    float jumpForce;

    PlayerPose pose;
    bool isLookingRight;

    PlatformerPlayer(float moveSpeed=3000.f, float jumpForce=800.f, PlayerPose defaultPose=POSE_IDLE);
};

struct AnimationPlayer {
    const SheetAnimation* curAnim;
    int curFrame;
    float timer;

    AnimationPlayer(const SheetAnimation& startAnim);

    void SetAnimation(const SheetAnimation& anim);
};

struct AddedEntityGroup : EntityGroup {
    AddedEntityGroup(float gravity);
};

// TODO: rewrite in ECS
struct PlayerFPS {
    Camera camera;
    float eyeHeight;
    bool isFocused;

    float angle;
    float tilt;

    Vector3 feet;

    PlayerFPS(float eyeHeight=1.8f);
    Vector3 Update(void* group, float delta);

    void Focus();
    void Unfocus();
    void Teleport(Vector3 position);

    void SetAngle(float lookAtDeg);
    void SetFov(float fovDeb);
};

void UpdateExtendedComponent(EntityGroup& group, IteratedComp& comp, float delta);
void DrawExtendedComponent(EntityGroup& group, IteratedComp& comp);
void UpdateExtendedGroup(EntityGroup& group, float delta);

// component-independent entity functions
void TranslateEntity(EntityID id, Vector2 offset);

#ifdef MAGMA_3D
void TranslateEntity(EntityID id, Vector3 offset);
inline void TranslateEntity(EntityID id, float x, float y, float z) {
    Vector3 offset = { x,y,z };
    TranslateEntity(id, offset);
}
#endif

inline void TranslateEntity(EntityID id, float x, float y){
    Vector2 offset = {x,y};
    TranslateEntity(id, offset);
}

void SetEntityCenter(EntityID id, Vector2 pos);
#ifdef MAGMA_3D
void SetEntityCenter(EntityID id, Vector3 pos);
inline void SetEntityCenter(EntityID id, float x, float y, float z) {
    Vector3 offset = { x,y,z };
    SetEntityCenter(id, offset);
}

#endif
inline void SetEntityCenter(EntityID id, float x, float y){
    Vector2 offset = {x,y};
    SetEntityCenter(id, offset);
}

void SetEntitySize(EntityID id, Vector2 pos);
inline void SetEntitySize(EntityID id, float x, float y){
    Vector2 offset = {x,y};
    SetEntitySize(id, offset);
}

#ifdef MAGMA_3D
void SetEntitySize(EntityID id, Vector3 pos);
inline void SetEntitySize(EntityID id, float x, float y, float z){
    Vector3 offset = {x,y,z};
    SetEntitySize(id, offset);
}
#endif

#ifdef MAGMA_3D
inline void ResetEntityTranslation(EntityID id){
    Vector3 origin = Vector3Zero();
    SetEntityCenter(id,origin);
}
#endif

void SimplifyHitboxes(EntityGroup& group);
