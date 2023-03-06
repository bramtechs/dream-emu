#include "magma.h"

#include "magma_entity.hpp"
#include "Box2D/Box2D.h"

#include <filesystem>

// contains more "advanced" components and entities, not needed for some types of games
// can be removed from engine

void SimplifyHitboxes(EntityGroup& group){
    ERROR("NOT IMPLEMENTED");
}

PhysicsBody::PhysicsBody(float density, float friction){
    this->initialized = false;
    this->dynamic = true;
    this->density = density;
    this->friction = friction;
}
PhysicsBody::PhysicsBody(bool isDynamic) {
    this->initialized = false;
    this->dynamic = isDynamic;
    this->density = 0.f;
    this->friction = 0.f;
}

PhysicsBody::~PhysicsBody(){
}

Vector2 PhysicsBody::position(){
    Vector2 result = {};
    if (initialized){
        assert(body);
        auto pos = body->GetPosition();
        result.x = pos.x;
        result.y = pos.y;
    }
    return result;
}

float PhysicsBody::angle(){
    float angle = 0.f;
    if (initialized){
        assert(body);
        angle = body->GetAngle();
    }
    return angle;
}

PlatformerPlayer::PlatformerPlayer(PlatformerPlayerConfig& config) {
    this->moveSpeed = config.moveSpeed;
    this->jumpForce = config.jumpForce;
    this->pose = POSE_IDLE;

    for (int i = 0; i < PLAYER_POSE_COUNT; i++) {
        this->animations[i] = config.animations[i];
    }
}

AnimationPlayer::AnimationPlayer()
    : curFrame(0), curAnim(NULL), timer(0.f) {
}

AnimationPlayer::AnimationPlayer(const SheetAnimation& startAnim)
    : curFrame(0), curAnim(&startAnim), timer(0.f) {
}

void AnimationPlayer::SetAnimation(const SheetAnimation& anim) {
    // don't change animation if it already is playing
    if (curAnim == NULL || curAnim->name != anim.name) {
        curAnim = &anim;
        timer = 0.f;
        curFrame = 0;
    }
}

// ===== 3d =====
PlayerFPS::PlayerFPS(float eyeHeight) {
    this->eyeHeight = eyeHeight;
    this->isFocused = true;

    this->camera.projection = CAMERA_PERSPECTIVE;
    this->camera.up = { 0.0f, 1.0f, 0.f };

    SetAngle(0.f);
    SetFov(80.f);

    Focus();
}

void PlayerFPS::SetAngle(float lookAtDeg) {
    Vector3 pos = camera.position;
    Vector3 offset = {
        cosf(lookAtDeg * DEG2RAD),
        sinf(lookAtDeg * DEG2RAD),
        0
    };
    camera.target = Vector3Add(pos, offset);
}

void PlayerFPS::SetFov(float fovDeg) {
    camera.fovy = fovDeg;
}

// TODO shouldnt be a EntityGroup pointer but that doesnt compile for some reason
Vector3 PlayerFPS::Update(void* group, float delta) {

    // update camera
    UpdateCamera(&camera, isFocused ? CAMERA_CUSTOM:CAMERA_FREE);

    // snap to the floor 
    Ray ray = { 0 };

    Vector3 offset = { 0,this->eyeHeight,0 };
    ray.position = Vector3Add(this->camera.position, offset);
    ray.direction = { 0,-1,0 };

    RayCollision col = ((EntityGroup*)group)->GetRayCollision(ray);

    // move player to hit point
    feet = Vector3Add(col.point, offset);
    camera.position = this->feet;

    if (this->isFocused) {
        // TODO naive use proper quaternions instead!

        Vector2 mouse = Vector2Scale(Vector2Scale(GetMouseDelta(), delta), 3.f);
        tilt -= mouse.y;
        angle -= mouse.x;
        SetMousePosition(GetScreenWidth() * 0.5f, GetScreenHeight() * 0.5f);

        // look around
        Vector3 angle = {
            sinf(this->angle * DEG2RAD),
            sinf(this->tilt * DEG2RAD) * 2,
            cosf(this->angle * DEG2RAD)
        };

        // clamp
        tilt = Clamp(tilt, -90, 90);

        // movement
        float axisX = 0;
        bool pressed = false;
        if (IsKeyDown(KEY_W)) {
            axisX = 90;
            pressed = true;
        }
        if (IsKeyDown(KEY_S)) {
            axisX = 270;
            pressed = true;
        }
        if (pressed) {
            Vector3 offset = { cosf((this->angle - axisX) * DEG2RAD), 0, sinf((this->angle + axisX) * DEG2RAD) };
            camera.position = Vector3Add(camera.position, Vector3Scale(Vector3Scale(offset, delta), 10));
        }

        // look
        camera.target = Vector3Add(camera.position, angle);
    }

    // DrawText(TextFormat("%f\n%f\n\n%f\n%f\n%f horizontal movement not implemented lmao",player->angle,player->tilt,
    //                                                                        player->camera.position.x,
    //                                                                        player->camera.position.y,

    return col.point;
}

void PlayerFPS::Focus() {
    isFocused = true;
}

void PlayerFPS::Unfocus() {
    isFocused = false;
}

void PlayerFPS::Teleport(Vector3 pos) {
    camera.position = pos;
}

// pre-made entity composition 
EntityID SpawnPlatformerPlayer(Vector2 pos, PlatformerPlayerConfig config) {
    EntityID id = Group.AddEntity();

    Sprite sprite = Sprite({pos.x, pos.y});
    Texture foxTexture = RequestTexture("spr_player_fox");
    sprite.SetTexture(foxTexture);
    Group.AddEntityComponent(id, COMP_SPRITE, sprite, true);

    AnimationPlayer animPlayer = AnimationPlayer();
    Group.AddEntityComponent(id, COMP_ANIM_PLAYER,animPlayer);

    PhysicsBody body = PhysicsBody(30.f,0.5f);
    Group.AddEntityComponent(id, COMP_PHYS_BODY,body);

    PlatformerPlayer player = PlatformerPlayer(config);
    Group.AddEntityComponent(id, COMP_PLAT_PLAYER,player, true);

    return id;
}
