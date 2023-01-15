#include "prefs.h"

UserPrefs Prefs = { 0 };

void SaveUserPrefs(){
    if (CreateDirectory("../save")){
        if (SaveFileData(PREFS_PATH, &Prefs, sizeof(UserPrefs))){
            DEBUG("Saved engine settings!");
            return;
        }
    }
    ERROR("Could not save engine settings!");
}

void LoadUserPrefs(){
    if (!DirectoryExists("../save") || !FileExists(PREFS_PATH)){
        DEBUG("No engine settings found, using defaults...");
        return;
    }

    DEBUG("Loading engine settings from file...");
    unsigned int size = 0;
    unsigned char* data = LoadFileData(PREFS_PATH, &size);

    memcpy(&Prefs,data,size);

    INFO("Loaded engine settings from file.");
    UnloadFileData(data);
}

