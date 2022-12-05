#define WATER_DEPTH 0.3f

void drawing_draw_floor(Block *block, Color color)
{
    DrawPlane(block->pos, {1.f, 1.f}, color);
}

void drawing_draw_floor(Block *block)
{
    Color tint;
    Texture *texture;
    Vector3 offset = Vector3Zero();
    switch (block->id)
    {
        case TILE_FLOOR_GRASS:
            tint = GREEN;
            texture = Assets->noiseTexture;
            break;
        case TILE_FLOOR_WATER:
            tint = BLUE;
            texture = Assets->noiseTexture;
            offset = {0.0f, -WATER_DEPTH};
            break;
        default:
            tint = WHITE;
            texture = Assets->placeHolderTexture;
            break;
    }

    Model floor = *Assets->floorModel;
    floor.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = *texture;
    Vector3 combPos = Vector3Add(block->pos, offset);
    DrawModel(floor, combPos, 1, tint);
}

void drawing_scene_draw(LevelLayout *layout)
{
    ClearBackground(layout->environment.skyColor);

    auto blocks = &layout->blocks;
    for (int i = 0; i < blocks->count; i++)
    {
        Block *block = blocks->get(i);
        drawing_draw_floor(block);
    }
}