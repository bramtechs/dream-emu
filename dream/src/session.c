typedef struct {
    Color skyColor;
    Color fogColor;
    float fogDistance;
} Environment;

Environment env_default(){
    Environment env = { 0 };
    env.skyColor = SKYBLUE;
    env.fogColor = GRAY;
    env.fogDistance = 0.2f;
    return env;
}

typedef struct {
    Environment env;
} Scene;

static Scene *CurrentScene;

#include "editor.c"

void session_init(void)
{
    CurrentScene = MemAlloc(sizeof(Scene));
    CurrentScene->env = env_default();
}

void session_update_and_render(float delta)
{
    ClearBackground(CurrentScene->env.skyColor);
    DrawGrid(10, 1);
}

void session_update_and_render_gui(float delta)
{
    editor_update_and_draw(CurrentScene);
}

void session_dispose()
{
    MemFree(CurrentScene);
    CurrentScene = NULL;
}
