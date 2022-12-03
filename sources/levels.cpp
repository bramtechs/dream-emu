static Color *palette = nullptr;
static int paletteCount = 0;


void level_fetch(const char* palettePath){
    Image img = LoadImage(palettePath);

    if (img.data == nullptr){
        TraceLog(LOG_FATAL,"Could not load palette!");
    }

    palette = LoadImagePalette(img,1024,&paletteCount);
    TraceLog(LOG_INFO,"Loaded %d palette indices...",paletteCount);

    UnloadImage(img);
}

bool color_equals(Color one, Color two){
    if (one.r != two.r)  return false;
    if (one.g != two.g)  return false;
    if (one.b != two.b)  return false;
    if (one.a != two.a)  return false;
    return true;
}

void level_data_load(LevelData *data, const char* texturePath){
    Image img = LoadImage(texturePath);
    if (palette == nullptr){
        level_fetch("assets/palette.png");
    }

    int width = data->width = img.width;
    int height = data->height = img.height;
    data->tiles = (int *) calloc(width * height, sizeof(int));

    // read each pixel
    for (int y = 0; y < height; y++){
        for (int x = 0; x < width; x++){
            Color color = GetImageColor(img,x,y);

            if (color.a == 0){
                continue;
            }

            // match color with database
            int match = -1;
            for (int i = 0; i < paletteCount; i++){
                if (color_equals(palette[i],color)){
                    match = i;
                    break;
                }
            }
            if (match == -1){
                // TODO lags console
                TraceLog(LOG_WARNING,"Unknown level data color!");
                continue;
            }

            // set index in level array
            int i = y * width + x;
            CurrentLevel->data.tiles[i] = match;
        }
    }
    TraceLog(LOG_INFO,"Loaded level!");

    UnloadImage(img);
}

void level_init001(GameLevel *level){
    level->environment.skyColor = SKYBLUE;
    level_data_load(&level->data,"assets/level001.png");
}

void level_dispose(){
    UnloadImagePalette(palette);
}