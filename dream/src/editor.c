#define UI_PADDING 40

void editor_update_and_draw(Scene* scene)
{
    int win_x = 10; 
    int win_y = 10; 
    int width = 480;

    Rectangle rect = {win_x, win_y, width, 700};
    if (!GuiWindowBox(rect, "Editor"))
    {
        rect.x += UI_PADDING;
        rect.y += UI_PADDING+20;
        rect.width = 150;
        rect.height = 150;

        scene->env.skyColor = GuiColorPicker(rect, "Sky color", scene->env.skyColor);

        rect.y += 150+UI_PADDING;
        rect.height = 20;

        char* fogStr = TextFormat("%f",scene->env.fogDistance);
        scene->env.fogDistance = GuiSlider(rect, "Fog", fogStr, scene->env.fogDistance, 0.f, 1.f);       // Slider control, returns selected value
    }
}
