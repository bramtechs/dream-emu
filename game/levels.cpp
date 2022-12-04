#include "shared.hpp"
#include "tiles.cpp"

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

extern "C" void level_load(void *data, short index)
{
    const char* texturePath = TextFormat("assets/levels/level%03d.png",index);
    auto *layout = (LevelLayout *) data;

    layout->environment.skyColor = RED;

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
            Color color = GetImageColor(img, x, y);

            if (color.a == 0)
            {
                continue;
            }

            // match color with database
            int match = -1;
            for (int i = 0; i < paletteCount; i++)
            {
                if (color_equals(palette[i], color))
                {
                    match = i;
                    break;
                }
            }
            if (match == -1)
            {
                // TODO lags console
                TraceLog(LOG_WARNING, "Unknown level layout color!");
                continue;
            }

            // set index in level array
            int i = y * width + x;
        }
    }
    TraceLog(LOG_INFO, "Loaded level!");

    UnloadImage(img);
}

extern "C" void level_update_and_stream(void *data)
{
    auto *layout = (LevelLayout *) data;
}