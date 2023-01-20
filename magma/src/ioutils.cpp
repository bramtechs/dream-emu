#include "magma.h"

MagmaSettings Settings = { 0 };

void SaveMagmaSettings(){
    if (CreateDirectory("../save")){
        if (SaveFileData(MAGMA_CONF_PATH, &Settings, sizeof(MagmaSettings))){
            DEBUG("Saved engine settings!");
            return;
        }
    }
    ERROR("Could not save engine settings!");
}

void LoadMagmaSettings(){
    if (!DirectoryExists("../save") || !FileExists(MAGMA_CONF_PATH)){
        DEBUG("No engine settings found, using defaults...");
        return;
    }

    DEBUG("Loading engine settings from file...");
    unsigned int size = 0;
    unsigned char* data = LoadFileData(MAGMA_CONF_PATH, &size);

    memcpy(&Settings,data,size);

    INFO("Loaded engine settings from file.");
    UnloadFileData(data);
}

