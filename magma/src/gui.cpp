#include "magma.h"

// === ButtonGroup ===
// do not combine ButtonGroup with PopMenu struct as we 
// might want different type of ui components, buttons with images etc...

// TODO: focus support if multiple ButtonGroups are on screen
ButtonGroup::ButtonGroup(){
    selected = 0;
    index = 0;
    count = 0;
    goingUp = false;
}

void ButtonGroup::reset(){
    count = index;
    index = 0;

}

void ButtonGroup::poll(){
    // move cursor up and down
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)){
        selected++;
        goingUp = false;

        // play sound
        Sound sound = RequestSound("sfx_core_hover");
        PlaySound(sound);

    }
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)){
        selected--;
        goingUp = true;

        // play sound
        Sound sound = RequestSound("sfx_core_hover");
        PlaySound(sound);
    }
    selected = Wrap(selected,0,count);
}

bool ButtonGroup::next() {
    index++;
    return selected == index-1;
}

bool ButtonGroup::skip() { // use to skip stuff like labels
    if (selected == index){
        selected += goingUp ? -1:1;
    }
    index++;
    return false;
}

// WARN: does not account for disabled buttons
bool ButtonGroup::IsButtonSelected(int* index){
    if (IsKeyPressed(KEY_ENTER)){
        *index = selected;
        return true;
    }
    return false;
}

// === PopMenu + config ===

// over-engineered system to allow one popup menu to be controlled at once
struct PopMenuFocus {
    uint id;
    int priority;
    float lastDrawn;

    PopMenuFocus(uint id, int priority)
        : id(id), priority(priority), lastDrawn(0.f) {
    }

    bool isFresh(){
        return GetTime() - lastDrawn < 0.1f;
    }
};
std::vector<PopMenuFocus> PopMenus; // keep track of focus

PopMenuConfig::PopMenuConfig(Color bgColor, Color fgColor, Color textColor)
        : backColor(bgColor), lineColor(fgColor), textColor(textColor){
}

PopMenu::PopMenu(int priority){
    this->config = PopMenuConfig();
    this->initialized = false;
    this->id = PopMenus.size();

    PopMenuFocus f(id,priority);
    PopMenus.push_back(f);
}

PopMenu::PopMenu(PopMenuConfig config, int priority){
    this->config = config;
    this->initialized = false;
    this->id = PopMenus.size(); 

    PopMenuFocus f(id, priority);
    PopMenus.push_back(f);
}

PopMenu::~PopMenu(){
    for (int i = 0; i < PopMenus.size(); i++){
        if (PopMenus[i].id == this->id){
            PopMenus.erase(PopMenus.begin()+i);
            return;
        }
    }
    assert(false);
}

void PopMenu::RenderPanel(){
    if (this->initialized) { // hide first frame
        Rectangle menuTarget = {
            topLeft.x,topLeft.y,
            size.x, size.y
        };
        DrawRectangleRec(menuTarget, BLACK);
        DrawRectangleLinesEx(menuTarget, 4.f, DARKGRAY);
    }

    this->size = {};
    this->group.reset();

    // register me as fresh
    for (auto &item : PopMenus){
        if (item.id == this->id) {
            item.lastDrawn = GetTime();
            return;
        }
    }
    assert(false);
}

void PopMenu::EndButtons(Vector2 panelPos) {
    // apply bottom padding also
    size.y += config.padding * 2;

    // calculate bounds for next frame
    topLeft.x = panelPos.x - size.x * 0.5f;
    topLeft.y = panelPos.y - size.y * 0.5f;

    this->initialized = true;

    // poll if focused
    if (IsInFocus()) {
        this->group.poll();
    }
}

void PopMenu::EndButtons(){
    Vector2 pos = {
        Window.gameSize.x * 0.5f,
        Window.gameSize.y * 0.5f
    };
    EndButtons(pos);
}

void PopMenu::DrawPopButton(const char* text, bool selectable, bool isBlank){
    Vector2 textPos = {
        topLeft.x+config.padding+config.arrowPadding*3.f,
        topLeft.y+size.y + config.padding,
    };

    Color actualColor = config.textColor;
    if (!selectable){
        actualColor = ColorBrightness(config.textColor,-0.3);
    }

    Vector2 textSize = MeasureTextEx(GetFontDefault(), text, config.fontSize, 3.0f);    // Measure string size for Font
    textSize.x += config.padding * 2 + config.arrowPadding;

    bool isSelected = isBlank ? group.skip() : group.next();
    if (isSelected) {
        Vector2 triPos = {
            topLeft.x+config.padding+config.arrowPadding,
            topLeft.y+size.y + config.padding + textSize.y * 0.5f,
        };
        if (this->initialized){
            DrawMenuTriangle(triPos, actualColor);
        }
    }

    if (this->initialized) {
        DrawRetroText(text,textPos.x,textPos.y,config.fontSize,actualColor);
    }

    if (size.x < textSize.x){
        size.x = textSize.x;
    }
    size.y += textSize.y;
}

void PopMenu::DrawMenuTriangle(Vector2 center, Color color){
    Vector2 vertices[3] = {
        {-1,-1},
        {-1, 1},
        { 1, 0},
    };

    float offsetX = 0.f;
    if (!config.arrowTumbleMode){
        offsetX = (sinf(GetTime()*config.arrowOscil)+1)*0.5f*config.arrowOscil;
    }

    for (int i = 0; i < 3; i++){
        if (config.arrowTumbleMode) {
            vertices[i].y *= (sinf(GetTime()*3.f)+1)*0.5f;
        }
        vertices[i].x = center.x+offsetX+(vertices[i].x*config.arrowScale);
        vertices[i].y = center.y+(vertices[i].y*config.arrowScale);
    }

    DrawTriangleStrip(vertices,3,color);
}

bool PopMenu::IsInFocus(){
    // get highest priority available
    int highestPriority = 0;
    for (auto &item : PopMenus) {
        if (item.isFresh() && item.priority > highestPriority){
            highestPriority = item.priority;
        }
    }

    // check if last fresh panel of highest priority is me
    std::vector<uint> ids;
    for (auto &item : PopMenus) {
        if (item.isFresh() && item.priority == highestPriority){
            ids.push_back(item.id);
        }
    }

    if (ids.size() == 0) return false;
    return ids[ids.size()-1] == this->id;
}

bool PopMenu::IsButtonSelected(int* index){
    if (IsInFocus()) {
        bool sel = group.IsButtonSelected(index);
        if (sel){
            // play sound
            Sound sound = RequestSound("sfx_core_confirm");
            PlaySound(sound);
        }
        return sel;
    }
    return false;
}

// === Pause menu ===
struct PauseMenuSession {
    bool isOpened = false;
    PopMenu menu = PopMenu(FOCUS_HIGH);
};

static PauseMenuSession PauseSession = PauseMenuSession();

static const char* toggle(bool on, const char* suffix, const char* yes, const char* no){
    return TextFormat("%s %s",on ? yes:no,suffix);
}

static const char* toggle(bool on, const char* suffix){
    return toggle(on,suffix,"Hide","Show");
}


// TODO: remove
static bool IsMuted = false;

void UpdateAndRenderPauseMenu(float delta, Color bgColor){
    if (IsKeyPressed(KEY_ESCAPE)){
        ToggleGamePaused();
    }
    if (!GameIsPaused()) return;

    // draw bg (if any)
    DrawRectangleRec(GetWindowBounds(),bgColor);

    PopMenu& menu = PauseSession.menu;
    menu.RenderPanel();
    menu.DrawPopButton("Continue", false);
    menu.DrawPopButton("Reload", false);
    menu.DrawPopButton(toggle(IsMuted,"audio","Play","Mute"));
    menu.DrawPopButton("Quit");
    menu.DrawPopButton("",false,true);

    // TODO: define debug flags
    menu.DrawPopButton("== DEV-TOOLS ==",false,true);
    menu.DrawPopButton(toggle(LoggerIsOpen(),"console"));
    menu.DrawPopButton(toggle(EditorIsOpen(),"editor"));
    menu.DrawPopButton("Dump asset info");

    // button actions
    int index = 0;
    if (menu.IsButtonSelected(&index))
    {
        switch (index){
            case 0: // continue
                break;
            case 1: // reload
                // TODO: implement
                break;
            case 2: // mute/unmute 
                IsMuted = !IsMuted;
                SetMasterVolume(IsMuted ? 0.f:1.f);
                break;
            case 3: // quit
                CloseWindow();
                break;
            case 6: // show/hide console
                ToggleLogger();
                break;
            case 7: // show/hide editor
                ToggleEditor();
                break;
            case 8:
                INFO("=========================");
                PrintAssetList();
                INFO("=========================");
                PrintAssetStats();
                INFO("=========================");
                OpenLogger();
                break;
        }
    }

    menu.EndButtons();
}

bool GameIsPaused(){
    return PauseSession.isOpened;
}

void PauseGame(){
    PauseSession.isOpened = true;

    // play sound
    Sound sound = RequestSound("sfx_core_pause");
    PlaySound(sound);
}

void UnpauseGame(){
    PauseSession.isOpened = false;

    // play sound
    Sound sound = RequestSound("sfx_core_unpause");
    PlaySound(sound);
}

bool ToggleGamePaused(){
    if (PauseSession.isOpened){
        UnpauseGame();
    }else{
        PauseGame();
    }
    return PauseSession.isOpened;
}

// === Main Menu ===
constexpr int FADE_IN  =0;
constexpr int DISPLAY  =1;
constexpr int FADE_OUT =2;

MainMenu::MainMenu(MainMenuConfig config, bool skipSplash) 
    : skipSplash(skipSplash), config(config) {

    // initialize session
    state = FADE_IN;
    curSplash = 0;
    timer = 0.f;
    alpha = 0.f;
    isDone = false;

    // load all textures
    bgTexture = RequestTexture(config.bgPath);
    for (const auto &splash : config.splashes) {
        Texture img = RequestTexture(splash.imgPath);
        splashTextures.push_back(img);
    }

    INFO("Booting main menu!");
}

void MainMenu::DrawScreenSaver(float delta) {
    ClearBackground(saveCol);
    Color* c = &saveCol;
    c->r += delta * 10.f;
    c->g -= delta * 10.f;
    c->b += delta * 20.f;
    c->a = 255;
}

void MainMenu::DrawBackground(Texture texture, Color tint) {
    Rectangle src = { 0, 0, texture.width, texture.height };
    Rectangle dest = { 0, 0, Window.winSize.x, Window.winSize.y };
    DrawTexturePro(texture, src, dest, Vector2Zero(), 0.f, tint);
}

bool MainMenu::UpdateAndDraw(float delta) {
    if (skipSplash || isDone || config.width == 0) { // skip if not booted
        return true;
    }

    float waitTime = FADE_DURATION;
    Texture texture = {};

    if (curSplash < config.splashes.size()) {
        texture = splashTextures[curSplash];
        switch (state) {
        case FADE_IN:
            alpha += delta / FADE_DURATION;
            break;
        case DISPLAY: {
                SplashScreen splash = config.splashes[curSplash];
                waitTime = splash.duration;
                alpha = 1.f;
            }
            break;
        case FADE_OUT:
            alpha -= delta / FADE_DURATION;
            break;
        default:
            assert(false);
            break;
        }

        if (timer > waitTime) {
            timer -= waitTime;
            state++;
            if (state > FADE_OUT) {
                state = FADE_IN;
                curSplash++;
            }
        }
        timer += delta;
    }
    else {
        alpha = 1.f;
        texture = bgTexture;
    }

    BeginMagmaDrawing();

    char alphaByte = Clamp(alpha * 255, 0, 255);
    Color tint = { 255, 255, 255, alphaByte };
    DrawBackground(texture, tint);

    if (curSplash >= config.splashes.size()) {
        DrawRetroText(config.title, 20, 20, 36, WHITE);
    }

    if (IsKeyPressed(KEY_ENTER)) {
        isDone = true;
    }

    EndMagmaDrawing();
    EndDrawing();

    return false;
}
