#pragma once

#include "magma.h"
#include "client.h"
#include "editor.h"
#include "prefs.h"

extern bool DoDrawGrid;

struct Environment {
    Color skyColor;
    Color fogColor;
    float fogDistance;

    Environment(Color sky, Color fog, float distance);
    Environment();
};

struct Scene {
    Environment env;
    PlayerFPS player;
    Vector3 spawnPoint;

    void reset();

    virtual void update(float delta) = 0;
    virtual void draw() = 0;

    void update_and_render(float delta);
    void update_and_render_gui(float delta);

    Scene();
    Scene(const char* fileName);

    EntityGroup group;
    void* editor;
};
