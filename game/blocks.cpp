// TODO create big lookup table with all block properties
bool block_needs_wall(int id)
{
    switch (id)
    {
        case TILE_FLOOR_WATER:
            return true;
        default:
            return false;
    }
}

// TODO implement block faces
Block blocks_spawn(int x, int y, int palID, int neighbours[4])
{
    assert(palID >= 0 && palID < 64);

    Block block = {};
    block.id = palID;
    block.pos = {(float) x, 0, (float) y};

    for (int i = 0; i < 4; i++)
    {
        if (block_needs_wall(neighbours[i]))
        {
            block.walls = 1 << neighbours[i];
        }
    }

    return block;
}