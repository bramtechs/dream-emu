void drawing_draw_floor(Block *block, Color color)
{
    DrawPlane(block->pos, {1.f, 1.f}, color);
}

void drawing_draw_walls_lower(Block *block)
{
    if ((1 & DIR_NORTH) == block->walls){
        Assets->wallLowerModel->materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = *Assets->placeHolderTexture;
        DrawModelEx(*Assets->wallLowerModel,Vector3Add(block->pos,{0,2,0}),{1,0,0},90,Vector3One(),WHITE);
    }
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
            drawing_draw_walls_lower(block);
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
    // TODO make function
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