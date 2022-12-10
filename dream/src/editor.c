
void editor_update_and_draw(void)
{
    Color sky = SKYBLUE;
    Rectangle rect = {10, 10, 120, 200};
    if (GuiWindowBox(rect, "Editor"))
    {
    }
    sky = GuiColorPicker((Rectangle){10, 300, 80, 120}, "Sky color", sky);
}