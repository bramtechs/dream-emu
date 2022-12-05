#include "shared.hpp"
#include "blocks.cpp"

// Raylib GPU draw functions cannot be called!

static Color *palette = nullptr;
static int paletteCount = 0;

void level_fetch(const char *palettePath)
{
    Image img = LoadImage(palettePath);

    if (img.data == nullptr)
    {
        TraceLog(LOG_FATAL, "Could not load palette!");
    }

    palette = LoadImagePalette(img, 1024, &paletteCount);
    TraceLog(LOG_INFO, "Loaded %d palette indices...", paletteCount);

    UnloadImage(img);
}

bool color_equals(Color one, Color two)
{
    if (one.r != two.r) return false;
    if (one.g != two.g) return false;
    if (one.b != two.b) return false;
    if (one.a != two.a) return false;
    return true;
}

int level_tile_id_get(int x, int y, Image palImg)
{
    if (x >= 0 && y < palImg.width && y >= 0 && y < palImg.height)
    {
        Color color = GetImageColor(palImg, x, y);

        if (color.a == 0)
            return TILE_NONE;

        // match color with database
        int match = TILE_NONE;
        for (int i = 0; i < paletteCount; i++)
        {
            if (color_equals(palette[i], color))
            {
                match = i;
                break;
            }
        }
        if (match == TILE_NONE)
        {
            TraceLog(LOG_WARNING, "Unknown level layout color!");
        }
        return match;
    }
    return TILE_NONE;
}

extern "C" void level_load(void *data, short index)
{
    const char *texturePath = TextFormat("assets/levels/level%03d.png", index);
    auto *layout = (LevelLayout *) data;

    layout->environment.skyColor = DARKGRAY;

    Image img = LoadImage(texturePath);
    if (palette == nullptr)
    {
        level_fetch("assets/palette.png");
    }

    int width = img.width;
    int height = img.height;

    // read each pixel
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            // match color with database
            int match = level_tile_id_get(x, y, img);
            if (match == TILE_NONE){
               continue;
            }

            // generate block
            int neighbors[4];
            neighbors[DIR_NORTH] = level_tile_id_get(x, y - 1, img);
            neighbors[DIR_EAST] = level_tile_id_get(x + 1, y, img);
            neighbors[DIR_SOUTH] = level_tile_id_get(x, y + 1, img);
            neighbors[DIR_WEST] = level_tile_id_get(x - 1, y, img);

            Block block = blocks_spawn(x, y, match, neighbors);
            layout->blocks.push(block);
        }
    }
    TraceLog(LOG_INFO, "Loaded level!");

    UnloadImage(img);
}

extern "C" void level_update_and_stream(void *data)
{
    auto *layout = (LevelLayout *) data;
}