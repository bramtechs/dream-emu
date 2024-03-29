#pragma once

// c(++) libraries
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>

// the all powerful raylib
#include "raylib.h"
#include "raymath.h"

// box2d cuz i can't math
#include <Box2D/Box2D.h>
#include <functional>

#define MAX(a, b) ((a)>(b)? (a) : (b))
#define MIN(a, b) ((a)<(b)? (a) : (b))
#define STRING( name ) #name

#define INFO(...)  TraceLog(LOG_INFO,__VA_ARGS__)
#define WARN(...)  TraceLog(LOG_WARNING,__VA_ARGS__)
#define WARNING(...)  TraceLog(LOG_WARNING,__VA_ARGS__)
#define DEBUG(...) TraceLog(LOG_DEBUG,__VA_ARGS__)
#define ERROR(...) TraceLog(LOG_ERROR,__VA_ARGS__)

#define TODO(M) TraceLog(LOG_FATAL,"TODO: %s", M); assert(false);

typedef unsigned int uint;
typedef uint EntityID;
typedef uint ItemType;
typedef int AssetType;

constexpr uint COMP_ALL = 0;
constexpr uint COMP_PERSISTENT = 10000;
constexpr uint COMP_BASE = 1;
constexpr uint COMP_SPRITE = 2;
constexpr uint COMP_MODEL_RENDERER = 3;

constexpr float PIXELS_PER_UNIT = 16;

#define MAGMA_CONF_PATH "../save/engine_conf.dat"
#define COLORS_PER_PALETTE 256 
#define PATH_MAX_LEN 128

#define ASSET_ANY             -2
#define ASSET_CUSTOM          -1
#define ASSET_TEXTURE          0
#define ASSET_MODEL            1
#define ASSET_SOUND            2
#define ASSET_FRAG_SHADER      3
#define ASSET_VERT_SHADER      4
#define ASSET_FONT             5
#define ASSET_MUSIC            6
#define ASSET_VIDEO            7

#define FOCUS_LOW              0 // use for menus that stay open to avoid softlock
#define FOCUS_NORMAL           5
#define FOCUS_HIGH            10
#define FOCUS_CRITICAL       100

#define BeginCouroutine() \
    static float CTARGET = 0.f; \
    static float CTIMER = 0.f; \
    CTIMER += delta

#define SleepCouroutine(X) \
    CTARGET += X; \
    if (CTIMER < CTARGET) goto cour_end

#define RestartCouroutine() \
    CTIMER = 0; \
    goto cour_end

#define EndCouroutine() cour_end: CTARGET = 0.f

#define MAX_SPLASHES 8
#define FADE_DURATION 1

#undef assert
inline void assert(bool cond = false) {
    if (!cond) {
        int* val = NULL;
        *val = 666;
    }
}

template<typename T>
inline T* NN(T* ptr){
    if (ptr == NULL) assert(false);
    return ptr;
}

extern bool IS_DEBUG;

struct BoundingBox2D {
    Vector2 min;
    Vector2 max;
};

struct MovingRectangle {
    float vx;
    float vy;
    float x;
    float y;
    float w;
    float h;

    MovingRectangle(Rectangle rect, Vector2 vel={});
};

struct Palette {
    char name[64];
    int colors[COLORS_PER_PALETTE * 3];

    Color GetColor(int index);
    Color GetIndexColor(int index);
    void MapImage(Image img);
    void DrawPreview(Rectangle region);
    int MapColorLoosely(Color color);
    int MapColor(Color color);
};

constexpr Palette INVALID_PALETTE = {
    "invalid",
    {
        255,0,0,
        255,0,255
    }
};

struct plm_t;
struct Video {
    plm_t* mpeg;
    uint8_t* frameData;

    float timeScale;
    Music audio;
};

Video LoadVideo(const char* fileName, const char* audioFileName=NULL);
Video LoadVideoFromMemory(uint8_t *bytes, size_t length, Music audio={});
void UnloadVideo(Video video, bool includeAudio=true);

bool VideoHasAudio(Video video);

// easy playback
void PlayAndDrawVideo(Video video, Rectangle dest, Color tint=WHITE);
void PlayAndDrawVideo(Video video, Vector2 pos, Color tint=WHITE);
inline void PlayAndDrawVideo(Video video, int posX, int posY, Color tint=WHITE) {
    Vector2 v2 = {(float)posX,(float)posY};
    PlayAndDrawVideo(video, v2, tint);
}

// advanced playback
void DrawVideoFrame(Video video, Rectangle dest, Color tint=WHITE);
void DrawVideoFrame(Video video, Vector2 pos, Color tint=WHITE);
inline void DrawVideoFrame(Video video, int posX, int posY, Color tint=WHITE) {
    Vector2 v2 = {(float)posX,(float)posY};
    DrawVideoFrame(video, v2,tint);
}
void PlayVideoAudio(Video video);

Image GetVideoFrame(Video video);
Texture GetVideoFrameTexture(Video video); // WARN: Dispose after use!
float GetVideoFrameRate(Video video);
void AdvanceVideo(Video video, float delta);
inline void AdvanceVideo(Video video){
    AdvanceVideo(video, GetFrameTime());
}

struct MagmaWindow {
    Vector2 gameSize;
    Vector2 winSize;
    RenderTexture2D renderTarget;

    Palette defaultPalette;
    bool hasDefaultPalette;

    float timeScale;
    float scale;
    bool unscaled;
    Vector2 scaledMouse;
};
extern MagmaWindow Window;

struct MagmaSettings {
    bool unlockFrameRate;
    bool skipIntro;
};
extern MagmaSettings Settings;

struct RawAsset {
    char path[PATH_MAX_LEN];
    int64_t size;
    char* data;
};

struct SplashScreen{
    const char imgPath[128];
    float duration;
};

struct ButtonGroup { // always assign as 'static'
    int count; // only known after one frame
    int selected;
    int index;
    bool goingUp; // dumb hack

    ButtonGroup();

    void reset(); // call right after static constructor
    void poll(bool useWASD=true); // navigate menu with keyboard
    void pollGrid(uint cols, bool useWASD=true); // navigate menu with keyboard (in four directions)
    bool next();
    bool skip(); // used to skip stuff, like labels

    bool IsButtonSelected(int* index); // WARN: does not account for inactive buttons,
                                       // you need to check these yourself
};

// TODO: Add texture support
struct PopMenuConfig {
    Color backColor;
    Color lineColor;
    Color textColor;

    int fontSize = 16;
    float padding = 10.f;

    float arrowPadding = 8.f;
    float arrowScale = 10.f;
    float arrowOscil = 0.f;
    bool arrowTumbleMode = false;

    PopMenuConfig(Color bgColor=BLACK, Color fgColor=WHITE, Color textColor=WHITE);
};

enum ButtonBehavior {
    BUTTON_ACTIVE,
    BUTTON_INACTIVE,
    BUTTON_SPACER,
};

struct Button {
    std::string text;
    ButtonBehavior behavior;
    std::function<void()> onClick;
    std::function<void()> onHover;
};

struct ButtonTable : std::vector<Button> {
    void AddButton(std::string text, ButtonBehavior behavior, std::function<void()> func, std::function<void()> hover=NULL);
    void AddButton(std::string text, std::function<void()> func, std::function<void()> hover=NULL);
    void AddSpacer(std::string text="");
};

struct PopMenu {
    uint id;

    Color actualTextColor;
    PopMenuConfig config;
    ButtonGroup group;
    Vector2 topLeft;
    Vector2 size;
    int buttonCount;
    bool initialized;

    PopMenu(PopMenuConfig config, int priority=FOCUS_NORMAL);
    PopMenu(int priority=FOCUS_NORMAL);
    ~PopMenu();

    void RenderPanel(Color overrideColor=BLANK);
    int DrawPopButton(const char* text, bool selectable=true, bool isBlank=false);
    void DrawPopButtons(ButtonTable& table);

    void EndButtons(Vector2 panelPos);
    void EndButtons();

    bool IsInFocus();
    bool IsButtonSelected(int* index);
    void ProcessSelectedButton(ButtonTable& table);
};

typedef bool (*LayoutMenuFunc)(float delta);
struct MainMenuConfig {
    int width;
    int height;

    std::vector<SplashScreen> splashes;

    const char* bgPath; // background image or shader!
    bool bgTiled;
    const char* title;
    const char* subTitle;
    LayoutMenuFunc layoutFunc;
};

// TODO: make flexible queue system instead: MenuSequence
struct MainMenu {
    MainMenuConfig config;

    bool skipSplash;
    bool isDone;
    float timer;
    float alpha;

    size_t state;
    size_t curSplash;

    std::vector<Texture> splashTextures;
    Texture bgTexture;
    Color saveCol;

    MainMenu(MainMenuConfig config, bool skipSplash=false);
    bool UpdateAndDraw(float delta);

private:
    void DrawScreenSaver(float delta);
    void DrawBackground(Texture texture, Color tint, bool tiled=false);
};


struct Base {
    BoundingBox bounds;
    Color tint;

    Base(Vector3 pos = {0,0,0}, Color tint = WHITE);

    void Translate(Vector3 offset);
    inline void Translate(float x, float y, float z) {
        Translate({ x,y,z });
    }
    inline void TranslateX(float x) {
        Translate({ x,0,0 });
    }
    inline void TranslateY(float y) {
        Translate({ 0,y,0 });
    }
    inline void TranslateZ(float z) {
        Translate({ 0,0,z });
    }

    void SetCenter(Vector3 pos);
    inline void SetCenter(float x, float y, float z){
        SetCenter({x,y,z});
    }

    void SetSize(Vector3 pos);
    inline void SetSize(float x, float y, float z){
        SetSize({x,y,z});
    }

    inline void ResetTranslation() {
        SetCenter(Vector3Zero());
    }

    RayCollision GetMouseRayCollision(Camera3D camera);

    Vector3 center();
    Vector3 size();
    Vector3 halfSize();
};

struct ModelRenderer{
    const char* model;
    bool accurate;
    Vector3 offset; // from base center

    ModelRenderer(const char* modelPath, Base* base);
};

struct Sprite {
    BoundingBox2D bounds;
    int zOrder;
    Color tint;

    bool hFlip;
    bool vFlip;
    bool isVisible;

    Texture texture;
    Rectangle srcRect;

    Sprite(Vector2 pos = {0,0},
           Color tint = WHITE, int zOrder = 0);

    void Translate(Vector2 offset);
    inline void Translate(float x, float y) {
        Translate({ x,y });
    }
    inline void TranslateX(float x) {
        Translate({ x,0 });
    }
    inline void TranslateY(float y) {
        Translate({ 0,y });
    }

    void SetCenter(Vector2 pos);
    inline void SetCenter(float x, float y){
        SetTopLeft({x,y});
    }

    void SetTopLeft(Vector2 pos);
    inline void SetTopLeft(float x, float y){
        SetSize({x,y});
    }
    void SetSize(Vector2 size);
    inline void SetSize(float x, float y){
        SetSize({x,y});
    }

    inline void ResetTranslation() {
        SetCenter({0.f,0.f});
    }

    RayCollision GetMouseRayCollision(Camera2D camera);

    void SetTexture(Texture texture, Rectangle srcRect={});
    void SetFlipped(bool hFlip, bool vFlip);
    void SetFlippedX(bool hFlip);
    void SetFlippedY(bool vFlip);

    void SetVisible(bool visible);
    inline void Show(){
        SetVisible(true);
    }
    inline void Hide(){
        SetVisible(false);
    }

    Rectangle region();
    Vector2 center();
    Vector2 size();
    Vector2 halfSize();
};

struct CompContainer {
    ItemType type;
    void* data;
    size_t size;
    bool persistent;
};

struct EntityGroup;
typedef std::pair<const EntityID, CompContainer> IteratedComp;
typedef void (*UpdateComponentFunc)(EntityGroup& group, IteratedComp& comp, float delta);
typedef void (*DrawComponentFunc)(EntityGroup& group, IteratedComp& comp);

struct EntityGroup {
    std::multimap<EntityID, CompContainer> comps;
    std::multimap<DrawComponentFunc,bool> drawers;
    std::vector<UpdateComponentFunc> updaters;

    uint nextEntity;
    uint entityCount; // FIX: change

    EntityGroup();

    RayCollision GetRayCollision(Ray ray);

    bool GetMousePickedBase(Camera camera, Base** result);
    bool GetMousePickedBaseEx(Camera camera, Base** result, RayCollision* col);

    // .comps -- data format
    // ====================
    // version: uint32_t (0 means always load)
    // component count: uint32_t
    // entity count: uint32_t (for checking)
    // === PER COMPONENT ===
    // entityid: uint32_t
    // comptype: ItemType uint32_t
    // compsize: uint64_t
    // compdata: void*

    void ClearGroup();
    bool LoadGroup(const char* fileName);
    bool SaveGroup(const char* fileName, uint version=0);

    EntityID AddEntity();
    void DestroyEntity(EntityID id);

    bool EntityExists(EntityID id);
    bool EntityHasComponent(EntityID id, ItemType type);

    bool IsEntityAtPos(Vector2 centerPos, EntityID* found=NULL);

    // TODO dispose functions
    void* AddEntityComponent(EntityID id, ItemType type, void* data, size_t size, bool persistent=false);
    template <typename T> // template fun and crazy time!
    inline T* AddEntityComponent(EntityID id, ItemType type, T data, bool persistent=false){
        return (T*) AddEntityComponent(id, type, (void*)&data, sizeof(T), persistent);
    }

    template <typename T>
    bool TryGetEntityComponent(EntityID id, ItemType filter, T** result) {
        auto items = comps.equal_range(id); // get all results
        for (auto it=items.first; it!=items.second; ++it){
            if (it->second.type == filter){
                *result = (T*) it->second.data;
                return true;
            }
        }
        *result = NULL;
        return false;
    }

    template <typename T>
    void GetEntityComponent(EntityID id, ItemType filter, T** result) { // get **or create** a component (asserts valid pointer)
        assert(filter != COMP_ALL);
        if (!TryGetEntityComponent(id, filter, result)){
            // lazily attach new component to entity
            T empty = {};
            *result = (T*) AddEntityComponent(id, filter, empty);
            DEBUG("Lazily created component %d on the fly!", filter);
        }
        assert(*result != NULL);
    }

    std::vector<CompContainer> GetEntityComponents(EntityID id, ItemType type = COMP_ALL);
    std::multimap<EntityID,CompContainer> GetComponents(ItemType type = COMP_ALL);

    void RegisterUpdater(UpdateComponentFunc updateFunc);
    void RegisterDrawer(DrawComponentFunc drawFunc, bool isDebug=false);

    void UpdateGroup(float delta);
    void DrawGroup();
    void DrawGroupDebug();
};

#include "magma_extra.h"

void* M_MemAlloc(size_t size);
void M_MemFree(void* ptr);
void CheckAllocations();

// math stuff
Vector3 Vector2ToVector3(Vector2 vec2);
Vector2 Vector3ToVector2(Vector3 vec3);
Vector2 Vector2Absolute(Vector2 v2);
Vector2 Vector2Snap(Vector2 v2, float gridSize);
Vector3 Vector3Absolute(Vector3 v3);
Vector3 Vector3Snap(Vector3 v3, float gridSize);

Color InvertColor(Color col, bool invertAlpha = false);
Color ColorLerp(Color src, Color dst, float factor);
Rectangle BoundingBoxToRect(BoundingBox box);
Rectangle BoundingBoxToRect(BoundingBox2D box);
float GetRectangleDiameter(Rectangle rec);
float GetRectangleDiameterSquared(Rectangle rec);
float SweptAABB(MovingRectangle source, MovingRectangle target, Vector2* normal);

bool LoadAssets();
void DisposeAssets();
bool ImportAssetPackage(const char* filePath);
std::vector<std::string> GetAssetPaths(AssetType type=ASSET_ANY);
std::vector<std::string> GetAssetNames(AssetType type=ASSET_ANY);
std::vector<std::string> GetTileNames();
size_t GetAssetCount();

Texture RequestTexture(const std::string& name);
Texture RequestPlaceholderTexture();
Texture RequestIndexedTexture(const std::string& name, Palette palette); // URGENT TODO: cache results
Texture RequestIndexedTexture(const std::string& name);

Image RequestImage(const std::string& name);
Shader RequestShader(const std::string& name);
Sound RequestSound(const std::string& name);
Music RequestMusic(const std::string& name); // WARN: not cached
Font RequestFont(const std::string& name);

Video RequestVideo(const std::string& name); // WARN: not cached

char* RequestCustom(const std::string& name, size_t* size, const char* ext=NULL); // NOTE: memory is disposed by DisposeAssets()

Palette RequestPalette(const std::string& name);
Palette ParsePalette(char* text, const char* name="unnamed");

Font GetRetroFont();

Model RequestModel(const std::string& name);
Model LoadOBJFromMemory(const char* fileName);

int GetAssetType(const char* name);

void PrintAssetStats();
void PrintAssetList();
bool IsAssetLoaded(const std::string& name);

void ShowFailScreen(const std::string& text); // do not run in game loop

void InitMagmaWindow(int gameWidth,int gameHeight, int winWidth, int winHeight, const char* title);
void InitMagmaWindow(int winWidth, int winHeight, const char* title);
void CloseMagmaWindow();

void RegisterArguments(int argc, char** argv);
bool IsRunningWithArguments(std::initializer_list<std::string> args);
inline bool IsRunningWithArgument(std::string arg){
    return IsRunningWithArguments({arg});
}
inline bool IsRunningWithArgumentPair(std::string shortArg, std::string longArg){
    return IsRunningWithArguments({shortArg, longArg});
}

Rectangle GetScreenBounds(); // rectangle of screen (x,y always 0,0)
Rectangle GetWindowBounds(); // rectangle of game window (x,y always 0,0)

void OpenSystemConsole();
void CloseSystemConsole();

void SetTimeScale(float scale);
float GetTimeScale();

void BeginMagmaDrawing();
void EndMagmaDrawing();

// only use these if you use the Editor
bool HasDefaultPalette();
void SetDefaultPalette(Palette palette);
void ClearDefaultPalette();
// ======

void BeginPaletteMode(Palette palette);
void EndPaletteMode();

void DrawCheckeredBackground(int tileSize, const char* text, Color color, Color altColor, Color highlightColor, Color textColor = WHITE);
void DrawBoundingBox(BoundingBox2D bounds, Color tint);
void DrawRetroText(const char* text, int posX, int posY, int fontSize=18, Color color=WHITE);
Vector2 MeasureRetroText(const char* text, int fontSize=18);

Ray GetWindowMouseRay(Camera3D camera);

Vector2 GetWindowMousePosition(Camera2D camera);
Vector2 GetWindowTopLeft();
Vector2 GetWindowCenter();

void MagmaLogger(int msgType, const char* text, va_list args);
void SetTraceLogAssertLevel(TraceLogLevel level);
void SetTraceLogOpenLevel(TraceLogLevel level);
void ClearLog();
void DrawLog(float offsetX, float offsetY, int fontSize, bool drawBG = true);
void UpdateAndDrawLog(float offsetX=10.f, float offsetY=10.f, int fontSize=16);
bool LoggerIsOpen();
void OpenLogger();
void CloseLogger();
bool ToggleLogger();

void SaveMagmaSettings();
void LoadMagmaSettings();
bool CreateDirectory(const char* path);

// gui
bool GameIsPaused();
void PauseGame();
void UnpauseGame();
bool ToggleGamePaused();

// gui keyboard
typedef void (*InputBoxEntered)(std::string& text);
bool ShowInputBox(const char* title, InputBoxEntered callback, const char* defText = "", uint minLength=1, uint maxLength=16);
void UpdateAndRenderInputBoxes(float delta);

// editor stuff
typedef EntityID (*EntityBuilderFunction)(EntityGroup& group, Vector3 pos);
void RegisterEntityBuilderEx(const char* name, EntityBuilderFunction func, bool isStock=false);
#define RegisterEntityBuilder(F) RegisterEntityBuilderEx(#F,F)

struct Description {
    std::string typeName;
    std::string info;
    Color color=WHITE;
};

typedef Description (*ComponentDescriptor)(void* data);
void RegisterComponentDescriptor(ItemType type, ComponentDescriptor func);
Description DescribeComponent (CompContainer cont);

void UpdateAndRenderEditor(Camera3D camera, EntityGroup& group, float delta);
void UpdateAndRenderEditor(Camera2D camera, EntityGroup& group, float delta);
void UpdateAndRenderEditorGUI(EntityGroup& group, Camera* camera, float delta);
bool EditorIsOpen();
void OpenEditor();
void CloseEditor();
bool ToggleEditor();

std::string GetTempDirectory();
