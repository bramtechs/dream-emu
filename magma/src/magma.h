#pragma once

// c(++) libraries
#include <iostream>
#include <vector>
#include <filesystem>
#include <map>
#include <memory>
#include <cassert>
#include <sstream>

// the all powerful raylib
#include "raylib.h"
#include "raymath.h"

// tooling
#include "deflated_assets.h"

#define MAX(a, b) ((a)>(b)? (a) : (b))
#define MIN(a, b) ((a)<(b)? (a) : (b))

#define new(X) (X*)M_MemAlloc(sizeof(X))

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

#define MAGMA_CONF_PATH "../save/engine_conf.dat"

#define MAX_ASSETS     512 
#define ASSET_CUSTOM   0
#define ASSET_TEXTURE  1
#define ASSET_MESH     2
#define ASSET_AUDIO    3

#define COMP_ALL                0
#define COMP_BASE               1
#define COMP_MODEL_RENDERER     2

#define COLORS_PER_PALETTE 256 

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

extern size_t Allocations;

struct MagmaWindow {
	Vector2 gameSize;
	Vector2 winSize;
	RenderTexture2D renderTarget;

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

struct Palette {
	const char name[64];
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

struct Assets {
    DeflationPack pack;

    std::map<std::string, Texture> textures;
    std::map<std::string, Model> models;

    Assets(const char* file);
    ~Assets();

    static Assets* Init(const char* file);
    static void Dispose();

	static Texture RequestTexture(const char* name);
	static Image RequestImage(const char* name);
	static Model RequestModel(const char* name);
	static Shader RequestShader(const char* name);
	static Palette RequestPalette(const char* name);

	static FilePathList IndexModels();
    static void EnterFailScreen(int width, int height); // do not run in game loop

private:
    static Palette ParsePalette(const char* text);
};

struct LogLine {
    TraceLogLevel level;
    std::string text;
};

struct SplashScreen{
    const char imgPath[128];
    float duration;
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
    EntityID id;
    BoundingBox bounds;
    Color tint;

    Base(EntityID id);
    Base(EntityID id, Vector3 pos, Color tint);

    void Translate(Vector3 offset);
    inline void TranslateX(float x);
    inline void TranslateY(float y);
    inline void TranslateZ(float z);
    inline void TranslateXYZ(float x, float y, float z);

    void SetCenter(Vector3 pos);
    inline void ResetTranslation();

	RayCollision GetMouseRayCollision(Camera camera);

    inline Vector3 center();
    inline Vector3 size();
    inline Vector3 halfSize();
};

struct ModelRenderer{
    EntityID id;
    const char* model;
    bool accurate;
    Vector3 offset; //from base center

    ModelRenderer(EntityID id, const char* modelPath, Base* base);
};

struct EntityGroup {
    uint entityCount;
    std::multimap<ItemType, void*> comps;

	RayCollision GetRayCollision(Ray ray);

	bool GetMousePickedBase(Camera camera, Base** result);
	bool GetMousePickedBaseEx(Camera camera, Base** result, RayCollision* col);

	void LoadGroup(const char* fileName);
	void SaveGroup(const char* fileName);

	EntityID AddEntity();

    // TODO dispose functions
	template <typename T>
	void AddEntityComponent(ItemType type, EntityID id, T data) {
        auto ptr = M_MemAlloc(sizeof(T));
        memcpy(ptr, &data, sizeof(T));
		comps.insert({ type, ptr });
	}

	void* GetEntityComponent(EntityID id, ItemType filter);

	size_t UpdateGroup(float delta);
	size_t DrawGroup(Camera camera, bool drawOutlines);
};

struct PlayerFPS {
    Camera camera;
    float eyeHeight;
    bool isFocused;

    float angle;
    float tilt;

    Vector3 feet;

    PlayerFPS(float eyeHeight=1.8f);
    Vector3 Update(void* group, float delta);

	void Focus();
	void Unfocus();
	void Teleport(Vector3 position);

    void SetAngle(float lookAtDeg);
    void SetFov(float fovDeb);
};

void* M_MemAlloc(size_t size);
void M_MemFree(void* ptr);
void CheckAllocations();

Color InvertColor(Color col, bool invertAlpha = false);
Color ColorLerp(Color src, Color dst, float factor);

void InitMagmaWindow(int gameWidth,int gameHeight, int winWidth, int winHeight, const char* title);
void InitMagmaWindow(int winWidth, int winHeight, const char* title);
void CloseMagmaWindow();

void BeginMagmaDrawing();
void EndMagmaDrawing();

void DrawCheckeredBackground(int tileSize, const char* text, Color color, Color altColor, Color highlightColor, Color textColor = WHITE);

Ray GetWindowMouseRay(Camera camera);
Vector2 GetWindowTopLeft();

void MagmaLogger(int msgType, const char* text, va_list args);
void ClearLog();
void DrawLog(float offsetX, float offsetY, int fontSize);
void UpdateAndDrawLog(float offsetX=10.f, float offsetY=10.f, int fontSize=14);

void SaveMagmaSettings();
void LoadMagmaSettings();
bool CreateDirectory(const char* path);
