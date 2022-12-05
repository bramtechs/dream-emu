// TODO create big lookup table with all block properties
bool block_needs_wall(int id, int neighbors[4])
{
    if (id == TILE_FLOOR_WATER)
    {
        return false;
    }

    for (int i = 0; i < 4; i++)
    {
        if (neighbors[i] == TILE_FLOOR_WATER)
        {
            return true;
        }
    }
    return false;
}

// TODO implement block faces
Block blocks_spawn(int x, int y, int palID, int neighbors[4])
{
    assert(palID >= 0 && palID < 64);

    Block block = {};
    block.id = palID;
    block.pos = {(float) x, 0, (float) y};
    block.isBlock = block_needs_wall(palID, neighbors);

    return block;
}