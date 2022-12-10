#include "editor.c"

void session_init(void)
{
}

void session_update_and_render(float delta)
{
    DrawGrid(10, 1);
}

void session_update_and_render_gui(float delta)
{
    editor_update_and_draw();
}

void session_dispose()
{

}