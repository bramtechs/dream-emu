use raylib::prelude::*;
use include_dir::{include_dir, Dir};

static ASSET_DIR: Dir<'_> = include_dir!("$CARGO_MANIFEST_DIR/core_assets");

fn main() {
    let (mut rl, thread) = raylib::init().size(640, 480).title("Hello, World").build();

    let test_texture_bytes = include_bytes!("spr_bricks_14.png");

    let test_image = Image::load_image_from_mem(
        ".png",
        &test_texture_bytes.to_vec(),
        test_texture_bytes.len() as i32,
    ).expect("Couldn't load texture");

    let test_texture = rl.load_texture_from_image(&thread, &test_image).expect("Couldn't load texture from img");

    // if you enable the `glob` feature, you can for files (and directories) using glob patterns
    let glob = "**/*.png";
    for entry in ASSET_DIR.find(glob).unwrap() {
        println!("Found {}", entry.path().display());
    }
    
    while !rl.window_should_close() {
        let mut d = rl.begin_drawing(&thread);

        d.draw_texture(&test_texture, 20, 20, Color::WHITE);

        d.clear_background(Color::WHITE);
        d.draw_text("Hello, world!", 12, 12, 20, Color::BLACK);
    }
}
