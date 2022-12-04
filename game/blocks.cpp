Block blocks_spawn(int x, int y, int palID)
{
    assert(palID >= 0 && palID < 64);

    Block block = {};
    block.id = palID;
    block.pos = {(float) x, 0, (float) y};

    return block;
}