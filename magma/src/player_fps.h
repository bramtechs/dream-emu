#pragma once

#include "raylib.h"
#include "raymath.h"
#include "entity.h"

typedef struct {
    Camera camera;
    float eyeHeight;
    bool isFocused;

    float angle;
    float tilt;

    Vector3 feet;
} PlayerFPS;

PlayerFPS SpawnPlayerFPS(float eyeHeight);

void SetPlayerFPSAngle(PlayerFPS *player, int lookAtDeg);
void SetPlayerFPSFov(PlayerFPS *player, int fovDeg);

Vector3 UpdatePlayerFPS(PlayerFPS* player, EntityGroup* group, float delta);

void FocusPlayerFPS(PlayerFPS *player);
void UnfocusPlayerFPS(PlayerFPS *player);

void TeleportPlayerFPS(PlayerFPS *player, Vector3 position);
