#include "player_fps.h"

PlayerFPS SpawnPlayerFPS(float eyeHeight){
    PlayerFPS player = { 0 };
    player.eyeHeight = eyeHeight;
    player.isFocused = true;

    Camera* cam = &player.camera;
    cam->projection = CAMERA_PERSPECTIVE;
    cam->up = (Vector3){ 0.0f, 1.0f, 0.f };

    SetCameraMode(*cam,CAMERA_CUSTOM);

    SetPlayerFPSAngle(&player,0);
    SetPlayerFPSFov(&player,80);

    FocusPlayerFPS(&player);
    return player;
}

void SetPlayerFPSAngle(PlayerFPS *player, int lookAtDeg){
    Vector3 pos = player->camera.position;
    Vector3 offset = {
        cosf(lookAtDeg*DEG2RAD),
        sinf(lookAtDeg*DEG2RAD),
        0
    };
    player->camera.target = Vector3Add(pos,offset);
}

void SetPlayerFPSFov(PlayerFPS *player, int fovDeg){
    player->camera.fovy = fovDeg;
}

Vector3 UpdatePlayerFPS(PlayerFPS* player, EntityGroup* group, float delta){

    // snap to the floor 
    Ray ray = { 0 };

    Vector3 offset = {0,player->eyeHeight,0};
    ray.position = Vector3Add(player->camera.position,offset);
    ray.direction = (Vector3) {0,-1,0};

    RayCollision col = GetRayCollisionGroup(group, ray);

    // move player to hit point
    player->feet = Vector3Add(col.point, offset);
    player->camera.position = player->feet;
    
    if (player->isFocused) {

        // TODO naive use proper quaternions instead!

        Vector2 mouse = Vector2Scale(Vector2Scale(GetMouseDelta(),delta),3.f);
        player->tilt -= mouse.y;
        player->angle -= mouse.x;
        SetMousePosition(GetScreenWidth()*0.5f,GetScreenHeight()*0.5f);

        // look around
        Vector3 angle = {
            sinf(player->angle * DEG2RAD),
            sinf(player->tilt * DEG2RAD)*2,
            cosf(player->angle * DEG2RAD)
        };

        // clamp
        player->tilt = Clamp(player->tilt,-90,90);

        // movement
        float axisX = 0;
        bool pressed = false;
        if (IsKeyDown(KEY_W)){
            axisX = 90;
            pressed = true;
        }
        if (IsKeyDown(KEY_S)){
            axisX = 270;
            pressed = true;
        }
        if (pressed){
            Vector3 offset = { cosf((player->angle - axisX) * DEG2RAD), 0, sinf((player->angle + axisX) * DEG2RAD) };
            player->camera.position = Vector3Add(player->camera.position, Vector3Scale(Vector3Scale(offset,delta),10));
        }

        // look
        player->camera.target = Vector3Add(player->camera.position,angle);
    }
    
    // DrawText(TextFormat("%f\n%f\n\n%f\n%f\n%f horizontal movement not implemented lmao",player->angle,player->tilt,
    //                                                                        player->camera.position.x,
    //                                                                        player->camera.position.y,

    return col.point;
}

void FocusPlayerFPS(PlayerFPS *player){
    player->isFocused = true;
    SetCameraMode(player->camera, CAMERA_CUSTOM);
}

void UnfocusPlayerFPS(PlayerFPS *player){
    player->isFocused = false;
}

void TeleportPlayerFPS(PlayerFPS *player, Vector3 pos){
    player->camera.position = pos;
}
