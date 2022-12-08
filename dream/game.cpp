#include "shared.hpp"

#include "logger.cpp"
#include "billboard.cpp"
#include "blocks.cpp"

static LevelLayout *Layout;
static LevelFeed *Feed;

bool color_equals(Color one, Color two)
{
    if (one.r != two.r) return false;
    if (one.g != two.g) return false;
    if (one.b != two.b) return false;
    if (one.a != two.a) return false;
    return true;
}

int level_tile_id_get(int x, int y)
{
    if (x >= 0 && y < POS(NN(Layout)->width) && y >= 0 && y < POS(Layout->height))
    {
        int j = y * Layout->width + x;
        Color color = Layout->colors[j];

        if (color.a == 0)
            return TILE_NONE;

        // match color with database
        int match = TILE_NONE;
        for (int i = 0; i < POS(Layout->paletteColorCount); i++)
        {
            if (color_equals(Layout->paletteColors[i], color))
            {
                match = i;
                break;
            }
        }
        assert (match != TILE_NONE);
        return match;
    }
    return TILE_NONE;
}

extern "C" void level_load(void *data, void *feed)
{
    Layout = (LevelLayout *) NN(data);
    Feed = (LevelFeed *) NN(feed);

    Feed->blocks.clear();

    Feed->environment.sunDirection = Vector3Normalize({1, -1, 1});
    Feed->environment.fogDensity = 0.0f;
    Feed->environment.skyColor = SKYBLUE;
    Feed->environment.sunColor = WHITE;

    // read each pixel
    for (int y = 0; y < POS(Layout->height); y++)
    {
        for (int x = 0; x < POS(Layout->width); x++)
        {
            // match color with database
            int match = level_tile_id_get(x, y);
            if (match == TILE_NONE)
                continue;

            // generate block
            int neighbors[4];
            neighbors[DIR_NORTH] = level_tile_id_get(x, y - 1);
            neighbors[DIR_EAST] = level_tile_id_get(x + 1, y);
            neighbors[DIR_SOUTH] = level_tile_id_get(x, y + 1);
            neighbors[DIR_WEST] = level_tile_id_get(x - 1, y);

            // TODO change
            if (match == TILE_BILL_TREE)
            {
                Billboard billboard = billboard_spawn(x, y, match);
                Feed->billboards.push(billboard);
            } else
            { // eww
                Block block = blocks_spawn(x, y, match, neighbors);
                Feed->blocks.push(block);
            }

        }
    }
    logger_log("Loaded level!");
}

extern "C" void level_update_and_stream(float delta)
{
    // make this move etc
}
