#include "settings.h"

UserSettings Settings = {
    #define CFG(s, n, default) default,
    #include "settings.def"
};

/* process a line of the INI file, storing valid values into config struct */
int handler(void *user, const char *section, const char *name,
            const char *value)
{
    UserSettings *cfg = (UserSettings *)user;

    if (0) ;
    #define CFG(s, n, default) else if (stricmp(section, #s)==0 && \
        stricmp(name, #n)==0) cfg->s##_##n = strdup(value);
    #include "settings.def"

    return 1;
}

/* print all the variables in the config, one per line */
void dump_config()
{
    #define CFG(s, n, default) printf("%s_%s = %s\n", #s, #n, Settings.s##_##n);
    #include "settings.def"
}

bool BOOL(char* ptr){
    return strcmp(ptr,"true") == 0;
}

void SetBoolean(char* ptr, bool enabled){
    if (BOOL(ptr) == enabled) return;
    char* asString = enabled ? "true" : "false";
    strcpy(ptr,asString);
}

void ToggleBoolean(char* ptr){
    bool on = !BOOL(ptr);
    SetBoolean(ptr,on);
}

void SaveUserSettings(){
    dump_config();
}

void LoadUserSettings(){

    const char* path = "save/settings.ini";
    //if (ini_parse(path, handler, &Settings) < 0){
    //    ERROR("Could not load engine .ini settings at %s",path);
    //    return;
    //}
    
    INFO("Loaded engine config from %s",path);
    return 0;
}
