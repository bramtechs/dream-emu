#include "magma.h"

static bool GUI_DEBUG = false;

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

void ButtonGroup::poll(bool useWASD){
    // move cursor up and down
    if ( IsKeyPressed(KEY_DOWN) || (useWASD && IsKeyPressed(KEY_S)) ){
        selected++;
        goingUp = false;

        // play sound
        Sound sound = RequestSound("sfx_core_hover");
        PlaySound(sound);

    }
    if ( IsKeyPressed(KEY_UP) || (useWASD && IsKeyPressed(KEY_W)) ){
        selected--;
        goingUp = true;

        // play sound
        Sound sound = RequestSound("sfx_core_hover");
        PlaySound(sound);
    }
    selected = Wrap(selected,0,count);
}

void ButtonGroup::pollGrid(uint cols, bool useWASD){
    // move cursor left and right
    if ( IsKeyPressed(KEY_RIGHT) || (useWASD && IsKeyPressed(KEY_D)) ){
        selected++;
        goingUp = false;

        // play sound
        Sound sound = RequestSound("sfx_core_hover");
        PlaySound(sound);
    }
    if ( IsKeyPressed(KEY_LEFT) || (useWASD && IsKeyPressed(KEY_A)) ){
        selected--;
        goingUp = true;

        // play sound
        Sound sound = RequestSound("sfx_core_hover");
        PlaySound(sound);
    }

    // vertical movement
    if ( IsKeyPressed(KEY_UP) || (useWASD && IsKeyPressed(KEY_W)) ){
        selected -= cols+1;
        goingUp = true;

        // play sound
        Sound sound = RequestSound("sfx_core_hover");
        PlaySound(sound);
    }
    if ( IsKeyPressed(KEY_DOWN) || (useWASD && IsKeyPressed(KEY_S)) ){
        selected += cols+1;
        goingUp = false;

        // play sound
        Sound sound = RequestSound("sfx_core_hover");
        PlaySound(sound);
    }
    selected = Wrap(selected,0,count);
}

bool ButtonGroup::next() {
    index++;
    int s = index - 1;
    // assert(selected >= 0 && s >= 0);
    return selected == s;
}

bool ButtonGroup::skip() { // use to skip stuff like labels
    if (selected == index){
        selected += goingUp ? -1:1;
    }
    index++;
    return false;
}

// WARN: does not account for inactive buttons
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

static void DrawMenuTriangle(Vector2 center, Color color=WHITE, float scale = 10.f,
                                                                float oscil = 0.5f,
                                                                bool tumbleMode=false){
    Vector2 vertices[3] = {
        {-1,-1},
        {-1, 1},
        { 1, 0},
    };

    float offsetX = 0.f;
    if (!tumbleMode){
        offsetX = (sinf(GetTime()*oscil)+1)*0.5f*oscil;
    }

    for (int i = 0; i < 3; i++){
        if (tumbleMode) {
            vertices[i].y *= (sinf(GetTime()*3.f)+1)*0.5f;
        }
        vertices[i].x = center.x+offsetX+(vertices[i].x*scale);
        vertices[i].y = center.y+(vertices[i].y*scale);
    }

    DrawTriangleStrip(vertices,3,color);
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

void ButtonTable::AddButton(std::string text, ButtonBehavior behavior, std::function<void()> func,
                                                                       std::function<void()> hover){
    Button but = { text, behavior, func, hover };
    emplace_back(but);
}

void ButtonTable::AddButton(std::string text, std::function<void()> func, std::function<void()> hover){
    Button but = {text, BUTTON_ACTIVE, func, hover};
    emplace_back(but);
}

void ButtonTable::AddSpacer(std::string text){
    Button but = {text, BUTTON_SPACER, NULL, NULL};
    emplace_back(but);
}

PopMenu::PopMenu(int priority){
    this->config = PopMenuConfig();
    this->initialized = false;
    this->id = PopMenus.size();
    this->group = ButtonGroup();

    PopMenuFocus f(id,priority);
    PopMenus.push_back(f);
}

PopMenu::PopMenu(PopMenuConfig config, int priority){
    this->config = config;
    this->initialized = false;
    this->id = PopMenus.size();
    this->group = ButtonGroup();

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

void PopMenu::RenderPanel(Color overrideColor){
    if (this->initialized) { // hide first frame
        Rectangle menuTarget = {
            topLeft.x,topLeft.y,
            size.x, size.y
        };

        actualTextColor = overrideColor.a == 0 ? config.textColor:overrideColor;
        Color borderColor = overrideColor.a == 0 ? config.lineColor:overrideColor;
        DrawPanel(menuTarget, config.backColor, borderColor);

        if (GUI_DEBUG) {
            const char* f = TextFormat("%d", this->group.selected);
            DrawRetroText(f,menuTarget.x-30, menuTarget.y-15, 14, YELLOW);
        }
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

    Color actualColor = actualTextColor; // nice variable names
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
        if (GUI_DEBUG) {
            const char* indexText = TextFormat("%d", buttonCount);
            DrawRetroText(indexText, textPos.x - 50, textPos.y, 14, isSelected ? GREEN : RED);
        }
    }

    if (size.x < textSize.x){
        size.x = textSize.x;
    }
    size.y += textSize.y;

    return buttonCount++;
}

void PopMenu::DrawPopButtons(ButtonTable& table) {
    for (const auto& but : table) {
        DrawPopButton(but.text.c_str(), but.behavior == BUTTON_ACTIVE, but.behavior == BUTTON_SPACER);
    }
}

void PopMenu::ProcessSelectedButton(ButtonTable& table) {
    int index = 0;
    if (IsButtonSelected(&index)) {
        if (index >= 0 && index < table.size()) {
            auto& but = table[index];
            if (but.behavior == BUTTON_ACTIVE && but.onClick){
                but.onClick();
            }
        }
    }
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
};

static PauseMenuSession PauseSession = PauseMenuSession();

// TODO: remove
static bool IsMuted = false;

void UpdateAndRenderPauseMenu(float delta, Color bgColor, AdvEntityGroup* group){
    if (IsKeyPressed(KEY_ESCAPE)){
        ToggleGamePaused();
    }
    if (!GameIsPaused()) return;

    // draw bg (if any)
    DrawRectangleRec(GetWindowBounds(),bgColor);

    ButtonTable buttons;
    ButtonBehavior behavior = group == NULL ? BUTTON_INACTIVE:BUTTON_ACTIVE;

    buttons.AddButton("Continue", NULL);
    buttons.AddButton("Reload", NULL);
    buttons.AddButton(IsMuted ? "Play audio":"Mute audio", [](){
        IsMuted = !IsMuted;
        SetMasterVolume(IsMuted ? 0.f:1.f);
    });
    buttons.AddButton("Quit",behavior,[](){
        CloseWindow();
    });

    buttons.AddSpacer();

    buttons.AddSpacer("== DEV-TOOLS ==");
    buttons.AddButton("Load level",behavior,[group](){
        group->LoadGroupInteractively();
        ToggleGamePaused();
    });
    buttons.AddButton("Clear level",behavior,[group](){
        group->ClearGroup();
    });
    buttons.AddButton("Export level",behavior,[group](){
        group->SaveGroupInteractively("raw_assets");
    });

    buttons.AddButton(LoggerIsOpen() ? "Hide console":"Show console",[](){
        ToggleLogger();
    });
    buttons.AddButton(EditorIsOpen() ? "Hide editor":"Open editor",[](){
        ToggleEditor();
    });
    buttons.AddButton("Dump asset info",[](){
        INFO("=========================");
        PrintAssetList();
        INFO("=========================");
        PrintAssetStats();
        INFO("=========================");
        OpenLogger();
    });

    static PopMenu menu = PopMenu(FOCUS_HIGH);
    menu.RenderPanel();
    menu.DrawPopButtons(buttons);
    menu.ProcessSelectedButton(buttons);
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

std::string ALPHABET_UPPER = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
std::string ALPHABET_LOWER = "abcdefghijklmnopqrstuvwxyz";
std::vector<std::string> ALPHABET_EXTRA = { "_", "Del", "Return" };

std::vector<std::string> CACHED_LOWER;
std::vector<std::string> CACHED_UPPER;

struct InputBox {
    int WIDTH = 400;
    int HEIGHT = 250;
    int PADDING = 15;
    int LETTERS_PER_ROW = 9;
    int FONT_SIZE = 18;

    bool isActive;
    std::string title;
    InputBoxEntered callback;
    uint minLength;
    uint maxLength;

    std::string curText;

    int enterKeyIndex;
    int deleteKeyIndex;

    ButtonGroup group;

    InputBox() {
        this->isActive = false;
        this->group.reset();
    }

    InputBox(const char* title, InputBoxEntered callback, const char* defText, uint minLength, uint maxLength){
        this->isActive = true;
        this->curText = defText;
        this->title = title;
        this->callback = callback;
        this->minLength = minLength;
        this->maxLength = maxLength;

        uint id = PopMenus.size();
        PopMenuFocus f(id,FOCUS_CRITICAL);
        PopMenus.push_back(f);

        // generate letter sets (if not done)
        if (CACHED_LOWER.empty()){
            for (const auto& letter : ALPHABET_LOWER){
                std::string str = {letter};
                CACHED_LOWER.push_back(str);
            }
            for (const auto& text : ALPHABET_EXTRA){
                CACHED_LOWER.push_back(text);
            }
        }

        if (CACHED_UPPER.empty()){
            for (const auto& letter : ALPHABET_UPPER){
                std::string str = {letter};
                CACHED_UPPER.push_back(str);
            }
            // add padding to create perfect grid
            for (int i = 0; i < 4; i++){
                CACHED_UPPER.push_back("");
            }
        }

        // determine index of special keys enter and delete
        deleteKeyIndex = CACHED_UPPER.size()+CACHED_LOWER.size()-2;
        enterKeyIndex  = CACHED_UPPER.size()+CACHED_LOWER.size()-1;
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

        float offsetX = MeasureRetroText(title.c_str(),FONT_SIZE).x;
        DrawRetroText(title.c_str(), topLeft.x + WIDTH * 0.5f - offsetX * 0.5f,
                                     topLeft.y + PADDING);

        // draw input text
        const int FLICKER_INTERVAL = 500;
        const char* displayedText = NULL;
        if (curText.length() < maxLength && ((int)(GetTime()*1000.f)) % (FLICKER_INTERVAL*2) > FLICKER_INTERVAL){
            displayedText = TextFormat("%s_",curText.c_str());
        } else {
            displayedText = curText.c_str();
        }

        DrawRetroText(displayedText, topLeft.x + PADDING + 20,
                                       topLeft.y + PADDING + 25);

        // draw letters
        this->group.reset();

        Rectangle lowerRegion = {
            topLeft.x + PADDING, topLeft.y + PADDING + 60,
            (float) WIDTH - PADDING*2, (float) 90
        };
        DrawLetterSet(lowerRegion, CACHED_UPPER);

        Rectangle upperRegion = lowerRegion;
        upperRegion.y += lowerRegion.height + PADDING * 0.2f;
        DrawLetterSet(upperRegion, CACHED_LOWER, CACHED_UPPER.size());

        // poll for input
        this->group.pollGrid(LETTERS_PER_ROW, false);

        // typing with keyboard (with what else lol)
        if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)){
            // TODO: prevent typing when pressing shift as it is buggy
        }else{
            char c = (char) GetKeyPressed();
            if (c == 1){ // pressed enter
                //INFO("%d",this->group.selected);
                if (this->group.selected == enterKeyIndex){ // pressed entered
                    AcceptInput();
                }else if (this->group.selected == deleteKeyIndex){ // pressed delete
                    EraseCharacter();
                }
                else { // put char
                    std::string s = GetTextWithIndex(this->group.selected);
                    if (!s.empty()){
                        PutCharacter(s[0]);
                    }
                }
            }
            else if (c == 3){ // pressed backspace
                EraseCharacter();
            }
            else if (c >= 32 && c <= 127){ // if pressed any useable ascii key + spacebar
                PutCharacter(c);
                DEBUG("pressed %c (%d)",c,c);
                // play type sound
                // TODO: this is annoying
                // TODO: play other sound when not accepted
                Sound sound = RequestSound("sfx_core_confirm");
                PlaySound(sound);
            }
        }
        
        //DrawRetroText(TextFormat("%d",this->group.selected),topLeft.x+20,topLeft.y+30,16,RED);
    }

    std::string GetTextWithIndex(int index){
        if (index < CACHED_UPPER.size()){
            return CACHED_UPPER[index];
        } else if (index < CACHED_UPPER.size() + CACHED_LOWER.size()){
            int i = index - CACHED_UPPER.size();
            if (CACHED_LOWER[i] == "_"){
                return " ";
            }
            return CACHED_LOWER[i];
        } else {
            return " ";
        }
    }

    void PutCharacter(char c){
        if (curText.length() < maxLength) {
            curText.push_back(c);
        }
    }

    void EraseCharacter(){
        if (curText.length() > 0){
            curText.pop_back();
        }
    }

    void AcceptInput(){
        isActive = false;
        if (callback != NULL){
            (*callback)(curText);
        }
        DEBUG("Accepted input: %s",curText.c_str());
    }

    void DrawLetterSet(Rectangle region, std::vector<std::string>& letters, uint offsetID=0) {
        Vector2 aCharSize = MeasureRetroText("a",FONT_SIZE);
        Vector2 charSize = {
            MAX((region.width - aCharSize.x) / LETTERS_PER_ROW, aCharSize.x),
            MAX((region.height - aCharSize.y) / LETTERS_PER_ROW, aCharSize.y)
        };

        int y = 0;
        int x = 0;
        for (int i = 0; i < letters.size(); i++){
            int xx = region.x + x * charSize.x;
            int yy = region.y + y * charSize.y;

            bool isSelected = this->group.selected-offsetID == i;
            //if (isSelected){
            //    Vector2 arrPos = {
            //        (float)xx - 8,
            //        (float)yy + FONT_SIZE * 0.5f - 2
            //    };
            //    DrawMenuTriangle(arrPos,WHITE,5.f);
            //}
            DrawRetroText(letters[i].c_str(), xx, yy,FONT_SIZE, isSelected ? GRAY:WHITE);
            this->group.next();

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
