#define TILE_FLOOR_GRASS 10
#define TILE_FLOOR_WATER 17

void tile_draw_floor(int x, int y, Color col)
{
    DrawPlane({(float) x, 0, (float) y}, {1, 1}, col);
}

void tile_draw(int x, int y, int index)
{
    assert(index >= 0 && index < 64);
    switch (index)
    {
        case TILE_FLOOR_GRASS:
            tile_draw_floor(x, y, GREEN);
            break;
        case TILE_FLOOR_WATER:
            tile_draw_floor(x, y, BLUE);
            break;

        default:
            break;
    }
}