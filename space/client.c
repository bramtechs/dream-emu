#define WIDTH 640
#define HEIGHT 480
#define SCALE 2

#include <iostream>
#include <cassert>
#include <vector>
#include "mem_array.cpp"

#include "raylib.h"
#include "raymath.h"

#define TILE_SIZE 16

// add component "bundles" or something, use unity like system

void rectangle_print(Rectangle rect){
    TraceLog(LOG_INFO,"%f %f %f %f",rect.x,rect.y,rect.width,rect.height);
}

struct Geometry {
    Vector2 pos;
    Vector2 size;
    Vector2 vel;

    Geometry(float x, float y, float w, float h){
        pos.x = x;
        pos.y = y;
        size.x = w;
        size.y = h;
    }

    Geometry(Vector2 pos, Vector2 size){
        this->pos = pos;
        this->size = size;
        this->vel = {};
    }

    Rectangle bounds(){
        return {pos.x,pos.y,size.x,size.y};
    }

    void update(float delta){
        pos.x += vel.x * delta;
        pos.y += vel.y * delta;
    }
    
};

struct SpriteRenderer {
    std::vector<Texture*> textures;
    float fps;
    float timer;

    SpriteRenderer(const char* name, float fps=5){
        //this->textures = assets_get_textures(name);
        this->fps = fps;
        this->timer = 0.f;
    }

    void update(float delta){
        this->timer += delta;
    }

    void draw(Geometry* geom){
        assert(geom->size.x > 0);
        assert(geom->size.y > 0);
        DrawRectangleV(geom->pos,geom->size,GREEN);
        TraceLog(LOG_INFO,"draw");
    }
};

struct Player {
    Geometry* geom;
    SpriteRenderer* sprite;

    Player(Vector2 pos){
        geom = new Geometry(pos,{24,48});
        sprite = new SpriteRenderer("hello",10);
    }

    void update(float delta){
        geom->update(delta);
        sprite->update(delta);
        rectangle_print(geom->bounds());
    }

    void draw(){
        sprite->draw(geom);
    }
};

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(WIDTH * SCALE, HEIGHT * SCALE, "SPACEGAME");
    SetWindowPosition((GetMonitorWidth(0) - WIDTH * SCALE) / 2, (GetMonitorHeight(0) - HEIGHT * SCALE) / 2);

    TraceLog(LOG_INFO, "Launched at %s", GetWorkingDirectory());

    RenderTexture2D target = LoadRenderTexture(WIDTH, HEIGHT);

    SetTargetFPS(60);

    Player player({20.f,20.f});

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        BeginDrawing();
        BeginTextureMode(target);

        float delta = GetFrameTime();

        ClearBackground(BLACK);
        DrawText("Hello world!", 50, 50, 16, WHITE);
        player.update(delta);
        player.draw();

        EndTextureMode();

        // TODO fix scuffed
        DrawTexturePro(target.texture, {0.0f, 0.0f, WIDTH, -HEIGHT}, {0.0f, 0.0f, WIDTH * SCALE, HEIGHT * SCALE},
                       {0.0f, 0.0f}, 0.0f, WHITE);

        EndDrawing();
    }

    UnloadRenderTexture(target);
    CloseWindow();                  // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
