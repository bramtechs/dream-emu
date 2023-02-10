#pragma once

// c(++) libraries
#include <iostream>
#include <vector>
#include <filesystem>
#include <map>
#include <unordered_map>
#include <memory>
#include <sstream>
#include <fstream>
#include <limits>

// the all powerful raylib
#include "raylib.h"
#include "raymath.h"

// box2d cuz i can't math
#include <Box2D/Box2D.h>

#define MAX(a, b) ((a)>(b)? (a) : (b))
#define MIN(a, b) ((a)<(b)? (a) : (b))
#define STRING( name ) #name

#ifdef __unix___ 
#define INFO(X...)  TraceLog(LOG_INFO,X)
#define WARN(X...)  TraceLog(LOG_WARNING,X)
#define DEBUG(X...) TraceLog(LOG_DEBUG,X)
#define ERROR(X...) TraceLog(LOG_ERROR,X)

#elif defined(_WIN32) || defined(WIN32)

#define INFO(...)  TraceLog(LOG_INFO,__VA_ARGS__)
#define WARN(...)  TraceLog(LOG_WARNING,__VA_ARGS__)
#define DEBUG(...) TraceLog(LOG_DEBUG,__VA_ARGS__)
#define ERROR(...) TraceLog(LOG_ERROR,__VA_ARGS__)
#endif

#define TODO(M) TraceLog(LOG_FATAL,"TODO: %s", M); assert(false);

constexpr int COMP_ALL = 0;
constexpr int COMP_BASE = 1;
constexpr int COMP_SPRITE = 2;
constexpr int COMP_MODEL_RENDERER = 3;

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

typedef unsigned int uint;
typedef uint ItemType;
typedef uint EntityID;

typedef int AssetType;

extern size_t Allocations;

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
    void poll(); // navigate menu with keyboard
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

struct PopMenu {
    uint id;

    PopMenuConfig config;
    ButtonGroup group;
    Vector2 topLeft;
    Vector2 size;
    bool initialized;

    PopMenu(PopMenuConfig config, int priority=FOCUS_NORMAL);
    PopMenu(int priority=FOCUS_NORMAL);
    ~PopMenu();

    void RenderPanel();
    void DrawPopButton(const char* text, bool selectable=true, bool isBlank=false);

    void EndButtons(Vector2 panelPos);
    void EndButtons();

    bool IsInFocus();
    bool IsButtonSelected(int* index);

private:
    void DrawMenuTriangle(Vector2 center, Color color=WHITE);
};

struct MainMenuConfig {
    int width;
    int height;

    std::vector<SplashScreen> splashes;

    const char* bgPath; // background image or shader!
    const char* title;
};

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
    void DrawBackground(Texture texture, Color tint);
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
        SetCenter(Vector2Zero());
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

struct ModelRenderer{
    const char* model;
    bool accurate;
    Vector3 offset; // from base center

    ModelRenderer(const char* modelPath, Base* base);
};

struct CompContainer {
    ItemType type;
    void* data;
};

struct EntityGroup {
    uint entityCount;
    std::multimap<EntityID, CompContainer> comps;
    b2World* world;

    // TODO: gravity should be in magma_extras
    EntityGroup(float gravity=9.8f);
    ~EntityGroup();

    RayCollision GetRayCollision(Ray ray);

    bool GetMousePickedBase(Camera camera, Base** result);
    bool GetMousePickedBaseEx(Camera camera, Base** result, RayCollision* col);

    void LoadGroup(const char* fileName);
    void SaveGroup(const char* fileName);

    EntityID AddEntity();

    // TODO dispose functions
    // TODO: try to not include in header
    template <typename T>
    void AddEntityComponent(ItemType type, EntityID id, T data){
        // make data stick with a malloc
        CompContainer cont;
        cont.type = type;
        cont.data = M_MemAlloc(sizeof(T));
        memcpy(cont.data, &data, sizeof(T));

        // add component in system
        comps.insert({id, cont});
    }

    void* TryGetEntityComponent(EntityID id, ItemType filter);
    inline void* GetEntityComponent(EntityID id, ItemType filter){
        return NN(TryGetEntityComponent(id, filter));
    }

    std::vector<CompContainer> GetEntityComponents(EntityID id, ItemType type = COMP_ALL);
    std::multimap<EntityID,void*> GetComponents(ItemType type = COMP_ALL);

    size_t UpdateGroup(float delta);

    size_t DrawGroup();
    size_t DrawGroupDebug(Camera3D camera);
    size_t DrawGroupDebug(Camera2D camera);
};

#include "magma_extra.h"

void* M_MemAlloc(size_t size);
void M_MemFree(void* ptr);
void CheckAllocations();

// math stuff
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
size_t GetAssetCount();

Texture RequestTexture(const std::string& name);
Texture RequestIndexedTexture(const std::string& name, Palette palette); // URGENT TODO: cache results
Texture RequestIndexedTexture(const std::string& name);

Image RequestImage(const std::string& name);
Model RequestModel(const std::string& name);
Shader RequestShader(const std::string& name);
Sound RequestSound(const std::string& name);
Font RequestFont(const std::string& name);
char* RequestCustom(const std::string& name, size_t* size, const char* ext=NULL); // NOTE: memory is disposed by DisposeAssets()
Palette RequestPalette(const std::string& name);
Palette ParsePalette(char* text, const char* name="unnamed");

Font GetRetroFont();

Model LoadOBJFromMemory(const char* fileName);

int GetAssetType(const char* name);

void PrintAssetStats();
void PrintAssetList();
bool IsAssetLoaded(const std::string& name);

void ShowFailScreen(const std::string& text); // do not run in game loop

void InitMagmaWindow(int gameWidth,int gameHeight, int winWidth, int winHeight, const char* title);
void InitMagmaWindow(int winWidth, int winHeight, const char* title);
void CloseMagmaWindow();
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
void UpdateAndRenderPauseMenu(float delta, Color bgColor=BLANK); // returns whether game should pause
bool GameIsPaused();
void PauseGame();
void UnpauseGame();
bool ToggleGamePaused();

// editor stuff
typedef EntityID (*EntityBuilderFunction)(EntityGroup& group, Vector3 pos);
void RegisterEntityBuilderEx(const char* name, EntityBuilderFunction func);
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
