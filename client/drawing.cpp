#include "rlights.h" // TODO shouldn't be needed

void drawing_draw_floor(Block *block, Texture *texture, Color tint, Vector3 offset = Vector3Zero())
{
    Model floorModel = *Assets->floorModel;
    floorModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = *texture;
    Vector3 pos = Vector3Add(block->pos, offset);
    DrawModel(floorModel, pos, 1, tint);
}

void drawing_draw_block(Block *block, Texture *texture, Color color, Vector3 offset = Vector3Zero())
{
    Model cubeModel = *Assets->cubeModel;
    Vector3 pos = Vector3Add(Vector3Add(block->pos, {0, -0.50, 0}), offset);
    cubeModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = *texture;
    DrawModel(*Assets->cubeModel, pos, 1.0f, color);
}

void drawing_draw(Block *block)
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
        case TILE_FLOOR_STONE:
            tint = GRAY;
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

    if (block->isBlock)
    {
        drawing_draw_block(block, texture, tint, offset);
    } else
    {
        drawing_draw_floor(block, texture, tint, offset);
    }
}