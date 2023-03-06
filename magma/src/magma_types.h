#pragma once
#include <map>
#include <vector>

#include "Box2D/Box2D.h"

struct EntityGroup;
extern EntityGroup Group;

typedef struct {
    BoundingBox bounds;
    Color tint;
} Base;

typedef struct {
    Model model;
    bool accurate;
    Vector3 offset; // from base center
} ModelRenderer;

typedef struct {
    BoundingBox bounds;
    Color tint;

    bool hFlip;
    bool vFlip;
    bool isVisible;

    Texture texture;
    Rectangle srcRect;
} Sprite;

typedef struct {
    bool initialized;
    bool dynamic;

    // TODO: find c physics

    float density;
    float friction;
} PhysicsBody;

typedef struct {
    float moveSpeed;
    float jumpForce;
    float gravityScale;

    PlayerPose pose;
    bool isLookingRight;

    const SheetAnimation* animations[PLAYER_POSE_COUNT];
} PlatformerPlayer;

typedef struct {
    const SheetAnimation* curAnim;
    int curFrame;
    float timer;

    AnimationPlayer();
    AnimationPlayer(const SheetAnimation& startAnim);

    void SetAnimation(const SheetAnimation& anim);
} AnimationPlayer;

typedef struct {
    EntityID id;
    CompType type;
    void* data;
    size_t size;
    bool persistent;
} CompContainer;

struct EntityGroup;
typedef void (*UpdateComponentFunc)(EntityID id, void* comp, float delta);
typedef void (*DrawComponentFunc)(EntityID id, void* comp);

#define MAX_COMPS 16000
#define MAX_COMP_TYPES 128

typedef struct {
    size_t count;
    CompContainer** comps;
} CompContainers;

typedef struct {
    CompContainers comps;

    size_t nextEntity;
    size_t entityCount;
    size_t compCount;

    DrawComponentFunc drawers[MAX_COMP_COUNT];
    DrawComponentFunc debugDrawers[MAX_COMP_COUNT];
    UpdateComponentFunc updaters[MAX_COMP_COUNT];

    union {
        Camera3D camera3D;
        Camera2D camera2D;
    };

    //void SaveGroupInteractively(const char* folder, uint version = 0); // TODO: migrate to editor.cpp!!
    //void LoadGroupInteractively(uint version = 0); // TODO: migrate to editor.cpp!!

    //bool IsHitboxAtPos(Vector2 centerPos, EntityID* id = NULL);
} EntityGroup;

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
