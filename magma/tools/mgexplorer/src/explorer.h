#include "deflated_assets.h"

struct Explorer {
    const int BAR_WIDTH = 150;

    DeflationPack* pack;
     
    Explorer(const char* filePath) {
        pack = new DeflationPack(filePath); 
    }

    ~Explorer(){
        delete pack;
    }

    void update_and_render(float delta) {
        ClearBackground(SKYBLUE);

        DrawRectangle(0,0,BAR_WIDTH,HEIGHT, RAYWHITE);
    }
};
