#include "assets.h"

Assets* _Assets_INIT(Assets* ptr){
    ptr->count = 10;
    return ptr;
}

Assets* assets_load(){
    Assets* assets = NEW(Assets,Assets_INIT);
    return assets;
}

void assets_dispose(Assets* assets){
    MemFree(assets);
    assets = NULL;
}
