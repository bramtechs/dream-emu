// contains more "advanced" components and entities, not needed for some types of games
// can be removed from engine
// TODO: Make optional with build flag

enum PlayerPose {
    POSE_IDLE,
    POSE_WALK_LEFT,
    POSE_WALK_RIGHT,
    POSE_JUMP,
    POSE_FALL,
    POSE_DUCK,
};

enum PlaybackMode {
    PLAY_LOOP,
    PLAY_PING_PONG,
    PLAY_ONCE,
};

constexpr char* PlayerPoseNames[] = {
    STRING(POSE_IDLE),
    STRING(POSE_WALK_LEFT),
    STRING(POSE_WALK_RIGHT),
    STRING(POSE_JUMP),
    STRING(POSE_FALL),
    STRING(POSE_JUMP),
};

constexpr int COMP_PLAT_PLAYER = 10;
constexpr int COMP_ANIM_PLAYER = 11;

struct SheetAnimation {
    std::string name; // description 
    std::string sheetName; // name of texture
    Vector2 origin; // top left of sector
    Vector2 cellSize; // size of each cell
    uint count; // number of cells in anim
    PlaybackMode mode;
    float fps;
};
struct PhysicsProps {
    Vector2 velocity;
    float maxSpeed;
    float accel;
    float damp;
    float gravity;

    PhysicsProps(float accel, float damp, float maxSpeed, float gravity=100.f);
};

// Sprite
struct PlatformerPlayer {
    PhysicsProps phys;
    PlayerPose pose;

    PlatformerPlayer(PhysicsProps physics, PlayerPose defaultPose=POSE_IDLE);
};

struct AnimationPlayer {
    SheetAnimation& curAnim;
    int curFrame;
    float timer;

    AnimationPlayer(SheetAnimation& startAnim);

    void SetAnimation(SheetAnimation& anim);
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

size_t UpdateGroupExtended(EntityGroup* group, float delta);
size_t DrawGroupExtended(EntityGroup* group);
