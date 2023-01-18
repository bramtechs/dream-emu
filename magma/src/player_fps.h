#pragma once

#include "raylib.h"
#include "raymath.h"
#include "entity.h"

struct PlayerFPS {
    Camera camera;
    float eyeHeight;
    bool isFocused;

    float angle;
    float tilt;

    Vector3 feet;

    PlayerFPS(float eyeHeight);
    Vector3 Update(EntityGroup* group, float delta);

	void Focus();
	void Unfocus();
	void Teleport(Vector3 position);

    void SetAngle(float lookAtDeg);
    void SetFov(float fovDeb);
};

