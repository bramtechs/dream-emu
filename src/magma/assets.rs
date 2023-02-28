use crate::info;
use include_dir::{include_dir, Dir};
use raylib::texture::Texture2D;

static CORE_ASSETS: Dir<'_> = include_dir!("$CARGO_MANIFEST_DIR/core_assets");
static GAME_ASSETS: Dir<'_> = include_dir!(env!("CARGO_MANIFEST_DIR"));
static ALL_ASSETS: [&'static Dir<'_>; 2] = [&CORE_ASSETS, &GAME_ASSETS];

static ASSET_TEXTURE: Vec<&str> = vec![".png", ".jpeg", ".bmp", ".gif"];
static ASSET_SOUND: Vec<&str> = vec![".mp3", ".ogg", ".wav", ".flac"];
static ASSET_FONT: Vec<&str> = vec![".png", ".jpeg", ".bmp", ".gif"];
static ASSET_FRAG_SHADER: Vec<&str> = vec![".fs"];
static ASSET_VERT_SHADER: Vec<&str> = vec![".vs"];
static ASSET_MODEL: Vec<&str> = vec![".obj", ".fbx"];

pub fn request_texture(name: &str) -> Option<Texture2D> {
    todo!();
}

fn count_of_type(extensions: &Vec<&'static str>) -> usize {
    let counter = 0;
    for ext in extensions {
        for folder in ALL_ASSETS {
            let glob = format!("**/*.{}", ext).as_str();
            for entry in folder.find(glob).unwrap() {
                counter += 1;
            }
        }
    }
    return counter;
}

pub fn print_asset_stats() {
    info!("Fonts {}", count_of_type(&ASSET_FONT));
    info!("Sounds {}", count_of_type(&ASSET_SOUND));
    info!("Vertex shaders{}", count_of_type(&ASSET_VERT_SHADER));
    info!("Fragment shaders{}", count_of_type(&ASSET_FRAG_SHADER));
    info!("Models {}", count_of_type(&ASSET_MODEL));
    info!("Textures {}", count_of_type(&ASSET_TEXTURE));
}

pub fn print_asset_list() {
    //for (const auto& item : Assets.assets) {
    //    const char* ext = GetFileExtension(item.path);
    //    const char* name = GetFileName(item.path);
    //    if (item.size >= 1000){
    //        int kbSize = item.size / 1000;
    //        TraceLog(LOG_INFO, "%s --> %s (size {} KB)", ext, name, kbSize);
    //    }else{
    //        TraceLog(LOG_INFO, "%s --> %s (size {} bytes)", ext, name, item.size);
    //    }
    //}
}
