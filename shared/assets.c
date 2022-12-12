#include "assets.h"

static Asset* create_asset(Assets *assets, const char* name, size_t size, AssetType type){
    Asset* asset = &assets->assets[assets->count];
    assets->count++;
    assert(assets->count < MAX_ASSETS);

    asset->type = type;
    asset->memory = M_MemAlloc(size);
    strcpy(asset->name,name);

    return asset;
}

static void load_file(Assets *assets, const char* name){
    if (IsFileExtension(name,".png")){
        Asset* asset = create_asset(assets,name,sizeof(Texture),AssetTexture);
        Texture texture = LoadTexture(name);
        // TODO add placeholder
        memcpy(asset->memory,&texture,sizeof(Texture));
    }
    else if (IsFileExtension(name,".wav")){

    }
}

Assets* LoadAssets(const char* folder){
    Assets* assets = new(Assets);
    strcpy(assets->folder,folder);

    INFO("Loading assets...");

    if (DirectoryExists(folder)){
        FilePathList list = LoadDirectoryFilesEx(folder, NULL, true);
        for (int i = 0; i < list.count; i++){
            const char *name = list.paths[i];
            load_file(assets,name);
        }
        UnloadDirectoryFiles(list);
    }else{
        ERROR("Asset directory not found!");
    }

    return assets;
}

void UnloadAssets(Assets* assets){
    M_MemFree(assets);
    INFO("Disposed assets!");
}

AssetList GetLoadedAssetList(Assets *assets){
    AssetList list = { 0 };
    list.count = assets->count;
    list.names = (char**) M_MemAlloc(assets->count*sizeof(char)*256);
    for (int i = 0; i < assets->count; i++){
       list.names[i] = assets->assets[i].name;
    }
    return list;
}

void UnloadAssetList(AssetList list){
    M_MemFree(list.names);
}
