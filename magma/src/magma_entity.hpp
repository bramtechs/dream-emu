#pragma once
#include <map>
#include <vector>

#include "Box2D/Box2D.h"

struct EntityGroup;
extern EntityGroup Group;

struct Base {
    BoundingBox bounds;
    Color tint;

    Base(Vector3 pos = {0,0,0}, Color tint = WHITE);

    void Translate(Vector3 offset);
    inline void Translate(float x, float y, float z) {
        Translate({ x,y,z });
    }
    inline void TranslateX(float x) {
        Translate({ x,0,0 });
    }
    inline void TranslateY(float y) {
        Translate({ 0,y,0 });
    }
    inline void TranslateZ(float z) {
        Translate({ 0,0,z });
    }

    void SetCenter(Vector3 pos);
    inline void SetCenter(float x, float y, float z){
        SetCenter({x,y,z});
    }

    void SetSize(Vector3 pos);
    inline void SetSize(float x, float y, float z){
        SetSize({x,y,z});
    }

    inline void ResetTranslation() {
        SetCenter(Vector3Zero());
    }

    RayCollision GetMouseRayCollision(Camera3D camera);

    Vector3 center();
    Vector3 size();
    Vector3 halfSize();
};

struct ModelRenderer{
    const char* model;
    bool accurate;
    Vector3 offset; // from base center

    ModelRenderer(const char* modelPath, Base* base);
};

struct Sprite {
    BoundingBox2D bounds;
    int zOrder;
    Color tint;

    bool hFlip;
    bool vFlip;
    bool isVisible;

    Texture texture;
    Rectangle srcRect;

    Sprite(Vector2 pos = {0,0},
           Color tint = WHITE, int zOrder = 0);

    void Translate(Vector2 offset);
    inline void Translate(float x, float y) {
        Translate({ x,y });
    }
    inline void TranslateX(float x) {
        Translate({ x,0 });
    }
    inline void TranslateY(float y) {
        Translate({ 0,y });
    }

    void SetCenter(Vector2 pos);
    inline void SetCenter(float x, float y){
        SetTopLeft({x,y});
    }

    void SetTopLeft(Vector2 pos);
    inline void SetTopLeft(float x, float y){
        SetSize({x,y});
    }
    void SetSize(Vector2 size);
    inline void SetSize(float x, float y){
        SetSize({x,y});
    }

    inline void ResetTranslation() {
        SetCenter({0.f,0.f});
    }

    RayCollision GetMouseRayCollision(Camera2D camera);

    void SetTexture(Texture texture, Rectangle srcRect={});
    void SetFlipped(bool hFlip, bool vFlip);
    void SetFlippedX(bool hFlip);
    void SetFlippedY(bool vFlip);

    void SetVisible(bool visible);
    inline void Show(){
        SetVisible(true);
    }
    inline void Hide(){
        SetVisible(false);
    }

    Rectangle region();
    Vector2 center();
    Vector2 size();
    Vector2 halfSize();
};

struct PhysicsBody {
    bool initialized;
    bool dynamic;

    float density;
    float friction;

    b2Body* body;

    PhysicsBody(float density = 30.f, float friction = 0.5f);
    PhysicsBody(bool isDynamic);
    ~PhysicsBody(); // TODO: dispose

    Vector2 position();
    float angle();
};

struct PlatformerPlayer {
    float moveSpeed;
    float jumpForce;
    float gravityScale;

    PlayerPose pose;
    bool isLookingRight;

    const SheetAnimation* animations[PLAYER_POSE_COUNT];

    PlatformerPlayer(PlatformerPlayerConfig& config);
};

struct AnimationPlayer {
    const SheetAnimation* curAnim;
    int curFrame;
    float timer;

    AnimationPlayer();
    AnimationPlayer(const SheetAnimation& startAnim);

    void SetAnimation(const SheetAnimation& anim);
};

// TODO: rewrite in ECS
struct PlayerFPS {
    Camera camera;
    float eyeHeight;
    bool isFocused;

    float angle;
    float tilt;

    Vector3 feet;

    PlayerFPS(float eyeHeight = 1.8f);
    Vector3 Update(void* group, float delta);

    void Focus();
    void Unfocus();
    void Teleport(Vector3 position);

    void SetAngle(float lookAtDeg);
    void SetFov(float fovDeb);
};

struct CompContainer {
    ItemType type;
    void* data;
    size_t size;
    bool persistent;
};

struct EntityGroup;
typedef std::pair<const EntityID, CompContainer> IteratedComp;
typedef void (*UpdateComponentFunc)(EntityGroup& group, IteratedComp& comp, float delta);
typedef void (*DrawComponentFunc)(EntityGroup& group, IteratedComp& comp);

struct EntityGroup {
    std::multimap<EntityID, CompContainer> comps;
    std::multimap<DrawComponentFunc,bool> drawers;
    std::vector<UpdateComponentFunc> updaters;

    union {
        Camera3D camera3D;
        Camera2D camera2D;
    };

    float gravity;
    b2World* world;

    uint nextEntity;
    uint entityCount; // FIX: change

    EntityGroup();
    ~EntityGroup();

    RayCollision GetRayCollision(Ray ray);

    bool GetMousePickedBase(Camera camera, Base** result);
    bool GetMousePickedBaseEx(Camera camera, Base** result, RayCollision* col);

    // .comps -- data format
    // ====================
    // version: uint32_t (0 means always load)
    // component count: uint32_t
    // entity count: uint32_t (for checking)
    // === PER COMPONENT ===
    // entityid: uint32_t
    // comptype: ItemType uint32_t
    // compsize: uint64_t
    // compdata: void*

    void ClearGroup();
    bool LoadGroup(const char* fileName);
    bool SaveGroup(const char* fileName, uint version=0);

    EntityID AddEntity();
    void DestroyEntity(EntityID id);

    bool EntityExists(EntityID id);
    bool EntityHasComponent(EntityID id, ItemType type);
    bool HasPhysics();

    bool IsEntityAtPos(Vector2 centerPos, EntityID* found=NULL);

    // TODO dispose functions
    void* AddEntityComponent(EntityID id, ItemType type, void* data, size_t size, bool persistent=false);
    template <typename T> // template fun and crazy time!
    inline T* AddEntityComponent(EntityID id, ItemType type, T data, bool persistent=false){
        return (T*) AddEntityComponent(id, type, (void*)&data, sizeof(T), persistent);
    }

    template <typename T>
    bool TryGetEntityComponent(EntityID id, ItemType filter, T** result) {
        auto items = comps.equal_range(id); // get all results
        for (auto it=items.first; it!=items.second; ++it){
            if (it->second.type == filter){
                *result = (T*) it->second.data;
                return true;
            }
        }
        *result = NULL;
        return false;
    }

    template <typename T>
    void GetEntityComponent(EntityID id, ItemType filter, T** result) { // get **or create** a component (asserts valid pointer)
        assert(filter != COMP_ALL);
        if (!TryGetEntityComponent(id, filter, result)){
            // lazily attach new component to entity
            T empty = {};
            *result = (T*) AddEntityComponent(id, filter, empty);
            DEBUG("Lazily created component %d on the fly!", filter);
        }
        assert(*result != NULL);
    }

    std::vector<CompContainer> GetEntityComponents(EntityID id, ItemType type = COMP_ALL);
    std::multimap<EntityID,CompContainer> GetComponents(ItemType type = COMP_ALL);

    void RegisterUpdater(UpdateComponentFunc updateFunc);
    void RegisterDrawer(DrawComponentFunc drawFunc, bool isDebug=false);

    void UpdateGroup(float delta);
    void DrawGroup();
    void DrawGroupDebug();

    void SaveGroupInteractively(const char* folder, uint version = 0); // TODO: migrate to editor.cpp!!
    void LoadGroupInteractively(uint version = 0); // TODO: migrate to editor.cpp!!

    bool IsHitboxAtPos(Vector2 centerPos, EntityID* id = NULL);
};
