void drawing_draw_floor(Block *block, Color color){
    DrawPlane(block->pos,{1.f,1.f},color);
}

void drawing_scene_draw(LevelLayout* layout){
    ClearBackground(layout->environment.skyColor);

    auto blocks = &layout->blocks;
    for (int i = 0; i < blocks->count; i++){
        Block* block = blocks->get(i);
        drawing_draw_floor(block,GREEN);
    }
}