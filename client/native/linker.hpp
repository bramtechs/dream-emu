#pragma once

#if _WIN32
typedef int (__cdecl *LEVEL_UPDATE_AND_STREAM)(float delta);
typedef int (__cdecl *LEVEL_LOAD)(void*,void*);

extern LEVEL_UPDATE_AND_STREAM level_update_and_stream;
extern LEVEL_LOAD level_load;
#endif

#if __linux__
typedef void (*LEVEL_UPDATE_AND_STREAM)(float);
typedef void (*LEVEL_LOAD)(void*,void*);

extern void (*level_update_and_stream)(float);
extern void (*level_load)(void*,void*);
#endif


bool linker_lib_link();
void linker_lib_free();
