#pragma once

typedef int (__cdecl *LEVEL_UPDATE_AND_STREAM)(float);
typedef int (__cdecl *LEVEL_LOAD)(void*,void*);

extern LEVEL_UPDATE_AND_STREAM level_update_and_stream;
extern LEVEL_LOAD level_load;

bool linker_lib_link();
void linker_lib_free();
