#include "assets.h"

static GameAssets *Assets = NULL;

void assets_load(const char *prefix)
{
    Assets = MemAlloc(sizeof(GameAssets));
    strcpy(Assets->prefix, prefix);
}

void assets_dispose()
{
    for (int i = 0; i < Assets->count; i++)
    {
        Asset *asset = &Assets->assets[i];
        switch (asset->type)
        {
            case TEXTURE_TYPE:
                UnloadTexture(*(Texture *) asset->memory);
                break;
            case SHADER_TYPE:
                UnloadShader(*(Shader *) asset->memory);
                break;

            default:
                assert(false);
        }
        MemFree(asset->memory);
    }
    MemFree(Assets);
}

void *assets_get(const char *name)
{
    assert(Assets);

    for (int i = 0; i < Assets->count; i++)
    {
        Asset *asset = &Assets->assets[i];
        assert(asset);
        if (strcmp(asset->name, name) == 0)
        {
            assert(asset->memory);
            return asset->memory;
        }
    }
    return NULL;
}

void assets_add(const char *name, char type, void *mem)
{
    assert(Assets);

    Asset *asset = &Assets->assets[Assets->count];
    strcpy(asset->name, name);
    asset->type = type;
    asset->memory = mem;

    Assets->count++;
}

Shader assets_shader(const char *name)
{
    void *mem = assets_get(name);
    if (mem != NULL)
    {
        const char *path = TextFormat("%s/%s", Assets->prefix, name);
        // TODO LoadShader()
    }
    if (mem != NULL)
    {
        return *(Shader *) mem;
    }
    assert(false);
}

Texture assets_texture(const char *name)
{
    assert(Assets);

    const char *path = TextFormat("%s/%s.png", Assets->prefix, name);

    Texture texture;

    void *mem = assets_get(name);
    if (mem != NULL)
    {
        texture = *(Texture *) mem;
    } else
    {
        mem = MemAlloc(sizeof(Texture));
        texture = LoadTexture(path);

        if (texture.id != 0)
        {
            INFO("Loaded texture %s (%s)", name, path);
        } else
        {
            WARN("Loaded placeholder texture %s (%s)", name, path);
            Image img = GenImageColor(64, 64, PINK);
            texture = LoadTextureFromImage(img);
            UnloadImage(img);
        }

        memcpy(mem, &texture, sizeof(Texture));
        assets_add(name, TEXTURE_TYPE, mem);
    }

    return *(Texture *) mem;
}
