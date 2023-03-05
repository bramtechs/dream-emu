#pragma once

const char* ALL_LEVELS[] = {
    "map_level001",
    "map_level001",
    "map_fluffoverload",
    "map_level001",
    "map_level001",
    "map_level001",
};

const char* MAP_SPRITE = "spr_map_placeholder";

struct LevelSelect {

    bool UpdateAndDraw(float delta, const char** selectedLevel);
};

#ifdef IMPLEMENT_TEMPLE_LEVEL_SELECT


bool LevelSelect::UpdateAndDraw(float delta, const char** selectedLevel){

    // draw main stuff
    BeginMagmaDrawing();

    ClearBackground(GOLD);

    // draw map
    Texture texture = RequestTexture(MAP_SPRITE);
    DrawTexture(texture, 0.f,0.f, WHITE);

    UpdateAndRenderPauseMenu(delta,{0,0,0,50});
    UpdateAndRenderInputBoxes(delta);

    // draw mouse coords
    Vector2 mouse = GetWindowMousePosition();
    DrawRetroText(TextFormat("x: %d\ny: %d",(int)mouse.x,(int)mouse.y),50,250,18,WHITE);

    EndMagmaDrawing();

    EndDrawing();

    *selectedLevel = ALL_LEVELS[0];

    return false;
}

#endif
