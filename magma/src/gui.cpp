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

// generic gui drawing helper functions
static void DrawPanel(Rectangle rect, Color bgColor=BLACK, Color borColor=WHITE){
    DrawRectangleRec(rect, bgColor);
    DrawRectangleLinesEx(rect, 4.f, borColor);
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
        DrawPanel(menuTarget);
    }

    this->buttonCount = 0;
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

int PopMenu::DrawPopButton(const char* text, bool selectable, bool isBlank){
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

    return buttonCount++;
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

// TODO: remove
static bool IsMuted = false;

void UpdateAndRenderPauseMenu(float delta, Color bgColor, EntityGroup* group){
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
    menu.DrawPopButton(IsMuted ? "Play audio":"Mute audio");
    menu.DrawPopButton("Quit");
    menu.DrawPopButton("",false,true);

    // TODO: define debug flags
    menu.DrawPopButton("== DEV-TOOLS ==",false,true);
    menu.DrawPopButton("Load level",group!=NULL);
    menu.DrawPopButton("Clear level",group!=NULL);
    menu.DrawPopButton("Export level",group!=NULL);
    menu.DrawPopButton(LoggerIsOpen() ? "Hide console":"Show console");
    menu.DrawPopButton(EditorIsOpen() ? "Hide editor":"Open editor");
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
            case 6: // import
                if (group)
                    group->LoadGroup("test.comps");
                break;
            case 7: // clear
                if (group)
                    group->ClearGroup();
                break;
            case 8: // export
                if (group){
                     group->SaveGroupInteractively("raw_assets");
                }
                break;
            case 9: // show/hide console
                ToggleLogger();
                break;
            case 10: // show/hide editor
                ToggleEditor();
                break;
            case 11:
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

void MainMenu::DrawBackground(Texture texture, Color tint, bool tiled) {
    Rectangle src = { 0.f, 0.f, (float) texture.width, (float) texture.height };
    Rectangle dest = { 0.f, 0.f, (float) Window.gameSize.x, (float) Window.gameSize.y };
    DrawTexturePro(texture, tiled ? dest:src, dest, Vector2Zero(), 0.f, tint);
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

    bool completed = curSplash >= config.splashes.size();

    unsigned char alphaByte = Clamp(alpha * 255, 0, 255);
    Color tint = { 255, 255, 255, alphaByte };
    DrawBackground(texture, tint, completed ? config.bgTiled : false);

    if (completed) {
        // draw menu elements
        if (config.title != NULL)
            DrawRetroText(config.title, 20, 20, 36, WHITE);
        if (config.subTitle != NULL)
            DrawRetroText(config.subTitle, 20, 60, 22, WHITE);
        // layout menu func
        if (config.layoutFunc != NULL){
            isDone = (*config.layoutFunc)(delta);
        }
    }

    if (config.layoutFunc == NULL && IsKeyPressed(KEY_ENTER)) {
        // default menu behavior
        isDone = true;
    }

    EndMagmaDrawing();
    EndDrawing();

    return false;
}

// virtual keyboard

std::string ALPHABET_LOWER = "abcdefghijklmnopqrstuvwxyz";
std::string ALPHABET_UPPER = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

struct InputBox {
    int WIDTH = 400;
    int HEIGHT = 250;
    int PADDING = 15;

    bool isActive;
    std::string title;
    std::string curText;
    InputBoxEntered callback;
    uint minLength;
    uint maxLength;

    InputBox() {
        this->isActive = false;
    }

    InputBox(const char* title, InputBoxEntered callback, const char* defText, uint minLength, uint maxLength){
        this->isActive = true;
        this->curText = defText;
        this->title = title;
        this->callback = callback;
        this->minLength = minLength;
        this->maxLength = maxLength;
    }

    void UpdateAndRender(float delta){
        Vector2 topLeft = {
            (float) Window.gameSize.x * 0.5f - WIDTH * 0.5f,
            (float) Window.gameSize.y * 0.5f - HEIGHT * 0.5f,
        };
        Rectangle region = {
            topLeft.x, topLeft.y,
            (float) WIDTH, (float) HEIGHT
        };

        DrawPanel(region);

        // draw title

        float offsetX = MeasureRetroText(title.c_str(),18).x;
        DrawRetroText(title.c_str(), topLeft.x + WIDTH * 0.5f - offsetX * 0.5f,
                                     topLeft.y + PADDING);

        // draw input text
        DrawRetroText(curText.c_str(), topLeft.x + PADDING + 20,
                                       topLeft.y + PADDING + 25);

        // draw letters
        Rectangle lowerRegion = {
            topLeft.x + PADDING, topLeft.y + PADDING + 60,
            (float) WIDTH - PADDING*2, (float) 90
        };
        DrawLetterSet(lowerRegion, ALPHABET_LOWER);

        Rectangle upperRegion = lowerRegion;
        upperRegion.y += lowerRegion.height + PADDING * 0.5f;
        DrawLetterSet(upperRegion, ALPHABET_UPPER);
    }

    void DrawLetterSet(Rectangle region, std::string& letters) {
        const int LETTERS_PER_ROW = 10;

        Vector2 aCharSize = MeasureRetroText("a",18);
        Vector2 charSize = {
            MAX((region.width - aCharSize.x) / LETTERS_PER_ROW, aCharSize.x),
            MAX((region.height - aCharSize.y) / LETTERS_PER_ROW, aCharSize.y)
        };

        int y = 0;
        int x = 0;
        for (int i = 0; i < letters.size(); i++){
            int xx = region.x + x * charSize.x;
            int yy = region.y + y * charSize.y;
            char text[2] = { letters[i], '\0' };
            DrawRetroText((const char*) &text, xx, yy);

            // go to next row
            x++;
            if (x > LETTERS_PER_ROW){
                x = 0;
                y++;
            }
        }
    }
};

static InputBox ActiveInputBox = InputBox();

bool ShowInputBox(const char* title, InputBoxEntered callback, const char* defText,
                                                    uint minLength, uint maxLength){
    if (ActiveInputBox.isActive){
        return false;
    }

    // set new active inputbox
    ActiveInputBox = InputBox(title, callback, defText, minLength, maxLength);
    return true;
}

void UpdateAndRenderInputBoxes(float delta){
    if (ActiveInputBox.isActive){
        ActiveInputBox.UpdateAndRender(delta);
    }
}
