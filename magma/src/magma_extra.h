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
    Vector2 velocity;
    bool dynamic;
    bool canCollide;
    float gravity;
    float maxSpeed;
    float damp;

    Rectangle curOverlap;

    PhysicsBody(bool dynamic=true, float gravity=100.f, float maxSpeed=1000.f, float damp=0.f);
};

struct PlatformerPlayer {
    float accel;
    PlayerPose pose;

    PlatformerPlayer(float accel=300.f, PlayerPose defaultPose=POSE_IDLE);
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
