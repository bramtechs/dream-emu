#include "settings.h"

MagmaSettings Settings = { 0 };

void SaveMagmaSettings(){
    ClearConfig(MAGMA_CONF_PATH);
    PutBool(MAGMA_CONF_PATH, "unlockFrameRate",Settings.unlockFrameRate);
    PutBool(MAGMA_CONF_PATH, "skipIntro",Settings.skipIntro);
}

void LoadMagmaSettings(){
}

void ClearConfig(const char* file){
    if (!SaveFileText(MAGMA_CONF_PATH,"")){
        ERROR("Failed to reset save config");
    }
}

inline void PutBool(const char* file, const char* name, bool on){
    PutField(file, name,on ? "true":"false");
}

void PutField(const char* file, const char* name, const char* value){
    int result = mkdir("../save", 0777);

    char* content = LoadFileText(file);
    const char* field = TextFormat("%s = %s",name,value);
    
    if (content != NULL){
        const char* newContent = TextFormat("%s\n%s",content,field);
        if (!SaveFileText(file, (char*) newContent)){
            ERROR("Failed to put field '%s' with data '%s' in file '%s'",name,value,file);
        }
    }else{
        if (!SaveFileText(file, (char*) field)){
            ERROR("Failed to put field '%s' with data '%s' in file '%s'",name,value,file);
        }
    }
    UnloadFileText(content);
}
