#include "magma.h"

// contains more "advanced" components and entities, not needed for some types of games
// can be removed from engine

PhysicsBody::PhysicsBody(bool dynamic, float gravity, float maxSpeed, float damp){
    this->dynamic = dynamic;
    this->gravity = gravity;
    this->maxSpeed = maxSpeed;
    this->damp = damp;
    this->velocity = {0,0};
    this->canCollide = true;
}

PlatformerPlayer::PlatformerPlayer(float accel, PlayerPose defaultPose) {
    this->accel = accel;
    this->pose = defaultPose;
}

AnimationPlayer::AnimationPlayer(SheetAnimation& startAnim)
    : curFrame(0), curAnim(startAnim), timer(0.f) {
}

void AnimationPlayer::SetAnimation(SheetAnimation& anim) {
    curAnim = anim;
    timer = 0.f;
    curFrame = 0;
}

size_t UpdateGroupExtended(EntityGroup* group, float delta){
    for (const auto& comp : group->comps) {
        switch (comp.second.type) {
        case COMP_ANIM_PLAYER:
        {
            auto animPlayer = (AnimationPlayer*)comp.second.data;
            auto sprite = (Sprite*) group->GetEntityComponent(comp.first, COMP_SPRITE);

            // advance current animation
            SheetAnimation& anim = animPlayer->curAnim;

            // set current frame
            Texture sheetTexture = RequestIndexedTexture(anim.sheetName);
            Rectangle src = {
                anim.origin.x + anim.cellSize.x * abs(animPlayer->curFrame),
                anim.origin.y,
                anim.cellSize.x,
                anim.cellSize.y,
            };

            sprite->SetTexture(sheetTexture,src);

            if (animPlayer->timer > 1.f/anim.fps){
                animPlayer->timer = 0.f;
                animPlayer->curFrame++;

                switch (anim.mode){
                    case PLAY_LOOP:
                        animPlayer->curFrame = Wrap(animPlayer->curFrame,0,anim.count-1);
                        break;
                    case PLAY_ONCE:
                        animPlayer->curFrame = Clamp(animPlayer->curFrame,0,anim.count-1);
                        break;
                    case PLAY_PING_PONG:
                        if (animPlayer->curFrame >= (int)anim.count){
                            animPlayer->curFrame = -anim.count+1;
                        }
                        break;
                }
            }
            animPlayer->timer += delta;
        } break;
        case COMP_PHYS_BODY: 
        {
            auto phys = (PhysicsBody*) comp.second.data;
            auto sprite = (Sprite*) group->GetEntityComponent(comp.first, COMP_SPRITE);

            if (sprite && phys->dynamic){
                // apply gravity -acceleration-
                if (!phys->isFloored) {
                    phys->velocity.y += phys->gravity * delta;
                }

                // apply damping
                phys->velocity.x += phys->velocity.x * delta * phys->damp;

                // apply speed limit 
                phys->velocity.x = Clamp(phys->velocity.x,-phys->maxSpeed,phys->maxSpeed);
                phys->velocity.y = Clamp(phys->velocity.y,-phys->maxSpeed,phys->maxSpeed);
                // =========================
                
                Rectangle targetPos = {
                    sprite->region().x + phys->velocity.x * delta,
                    sprite->region().y + phys->velocity.y * delta,
                    sprite->size().x,
                    sprite->size().y,
                };

                if (phys->canCollide) {
                    // collision detection
                    
                    // get biggest overlap between other PhysicsBodies
                    Rectangle overlap = {};
                    auto bodies = group->GetComponents(COMP_PHYS_BODY);
                    for (const auto &body : bodies){
                        auto otherSprite = (Sprite*) group->GetEntityComponent(body.first,COMP_SPRITE);
                        if (!sprite || body.first == comp.first) continue;
                        Rectangle colRec = GetCollisionRec(otherSprite->region(), sprite->region());
                        if (GetRectangleDiameterSquared(colRec) > GetRectangleDiameterSquared(overlap)){
                            overlap = colRec;
                        }
                    }
                    phys->curOverlap = overlap;
                    
                    // TODO: fix properly
                    // shift targetpos back
                    if (GetRectangleDiameterSquared(overlap) > EPSILON){
                        if (overlap.width < sprite->size().x) { 
                            // horizontal collision
                            phys->velocity.x = 0.f;
                            targetPos.x -= overlap.width;
                        }
                        if (overlap.height < sprite->size().y) {
                            // vertical collision
                            phys->velocity.y = 0.f;
                            targetPos.y -= overlap.height;

                            phys->isFloored = true; // TODO: handle ceiling
                        }
                    }
                    else {
                        phys->isFloored = false;
                    }
                }

                // =========================
                // apply -velocity-
                sprite->SetTopLeft(targetPos.x,targetPos.y);
            }
        } break;
        case COMP_PLAT_PLAYER:
        {
            auto plat = (PlatformerPlayer*) comp.second.data;
            auto sprite = (Sprite*) group->GetEntityComponent(comp.first,COMP_SPRITE);
            auto phys = (PhysicsBody*) group->GetEntityComponent(comp.first,COMP_PHYS_BODY);
        } break;
        default:
            break;
        }
    }
    return 0;
}

size_t DrawGroupExtended(EntityGroup* group){
    return 0;
}

// ===== 3d =====
PlayerFPS::PlayerFPS(float eyeHeight) {
    this->eyeHeight = eyeHeight;
    this->isFocused = true;

    Camera* cam = &this->camera;
    cam->projection = CAMERA_PERSPECTIVE;
    cam->up = { 0.0f, 1.0f, 0.f };

    SetCameraMode(*cam, CAMERA_CUSTOM);

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
    SetCameraMode(camera, CAMERA_CUSTOM);
}

void PlayerFPS::Unfocus() {
    isFocused = false;
}

void PlayerFPS::Teleport(Vector3 pos) {
    camera.position = pos;
}
