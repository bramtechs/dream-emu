#pragma once

// scuffed adapter to merge C++ with C game code

// the all powerful raylib
#include "raylib.h"
#include "raymath.h"

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

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

#define COMP_ALL                0
#define COMP_PERSISTENT     10000
#define COMP_BASE               1
#define COMP_SPRITE             2
#define COMP_MODEL_RENDERER     3

#define PIXELS_PER_UNIT        16

#define MAGMA_CONF_PATH "../save/engine_conf.dat"
#define PATH_MAX_LEN 128
#define STRING_COUNT 256
#define STRING_MAX_LEN 128

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
	inline void assert(bool cond) {
		if (!cond) {
			int* val = NULL;
			*val = 666;
		}
	}
	inline void panic() {
		assert(false);
	}

	extern bool IS_DEBUG;

	typedef struct {
		Vector2 min;
		Vector2 max;
	} BoundingBox2D;

	typedef int VideoID;
	typedef struct {
		VideoID id;
		uint8_t* frameData;

		float timeScale;
		Music audio;
	} Video;

	Video LoadVideo(const char* fileName);
	Video LoadVideoWithAudio(const char* fileName, const char* audioFileName);

	Video LoadVideoFromMemory(uint8_t* bytes, size_t length);
	Video LoadVideoWithAudioFromMemory(uint8_t* bytes, size_t length, Music music);

	void UnloadVideo(Video video, bool includeAudio);
	bool VideoHasAudio(Video video);

	// easy playback
	void PlayAndDrawVideoRec(Video video, Rectangle dest, Color tint);
	void PlayAndDrawVideoV(Video video, Vector2 pos, Color tint);
	void PlayAndDrawVideo(Video video, int posX, int posY, Color tint);

	// advanced playback
	void DrawVideoFrameRec(Video video, Rectangle dest, Color tint);
	void DrawVideoFrameV(Video video, Vector2 pos, Color tint);
	void DrawVideoFrame(Video video, int posX, int posY, Color tint);

	void PlayVideoAudio(Video video);

	Image GetVideoFrame(Video video);
	Texture GetVideoFrameTexture(Video video); // WARN: Dispose after use!
	float GetVideoFrameRate(Video video);
	void AdvanceVideo(Video video, float delta);

	typedef struct {
		Vector2 gameSize;
		Vector2 winSize;
		RenderTexture2D renderTarget;

		float timeScale;
		float scale;
		bool unscaled;
		Vector2 scaledMouse;
	} MagmaWindow;

	extern MagmaWindow Window;

	typedef struct {
		bool unlockFrameRate;
		bool skipIntro;
	} MagmaSettings;
	extern MagmaSettings Settings;

	// TODO: stack abuse (multiple kilobytes of data)
	typedef struct {
		size_t count;
		size_t capacity;
		char** entries;
	} StringArray;

	StringArray InitStringArrayEx(size_t capacity);
	StringArray InitStringArray();
	void AppendString(StringArray* array, const char* str);
	void UnloadStringArray(StringArray* array);

	// math stuff
	Vector3 Vector2ToVector3(Vector2 vec2);
	Vector2 Vector3ToVector2(Vector3 vec3);
	Vector2 Vector2Absolute(Vector2 v2);
	Vector2 Vector2Snap(Vector2 v2, float gridSize);
	Vector3 Vector3Absolute(Vector3 v3);
	Vector3 Vector3Snap(Vector3 v3, float gridSize);

	Color InvertColor(Color col);
	Color InvertColorWAlpha(Color col);
	Color ColorLerp(Color src, Color dst, float factor);
	Rectangle BoundingBoxToRect(BoundingBox box);
	Rectangle BoundingBox2DToRect(BoundingBox2D box);
	float GetRectangleDiameter(Rectangle rec);
	float GetRectangleDiameterSquared(Rectangle rec);

	// asset manager
	bool LoadAssets();
	void DisposeAssets();
	bool ImportAssetPackage(const char* filePath);
	StringArray GetFilteredAssetPaths(AssetType type);
	StringArray GetFilteredAssetNames(AssetType type);
	StringArray GetAssetPaths();
	StringArray GetAssetNames();
	StringArray GetTileNames();
	size_t GetAssetCount();

	Texture RequestTexture(const char* name);
	Texture RequestPlaceholderTexture();
	Image RequestImage(const char* name);
	Shader RequestShader(const char* name);
	Sound RequestSound(const char* name);
	Music RequestMusic(const char* name); // WARN: not cached
	Font RequestFont(const char* name);
	Video RequestVideo(const char* name); // WARN: not cached
	char* RequestCustom(const char* name, size_t* size, const char* ext); // NOTE: memory is disposed by DisposeAssets()
	Model RequestModel(const char* name);
	Font GetRetroFont();
	int GetAssetType(const char* name);
	void PrintAssetStats();
	void PrintAssetList();
	bool IsAssetLoaded(const char* name);

	void ShowFailScreen(const char* text); // do not run in game loop

	void InitMagmaWindow(int winWidth, int winHeight, const char* title);
	void InitMagmaWindowEx(int gameWidth, int gameHeight, int winWidth, int winHeight, const char* title);
	void CloseMagmaWindow();

	void RegisterArguments(int argc, char** argv);

	Rectangle GetScreenBounds(); // rectangle of screen (x,y always 0,0)
	Rectangle GetWindowBounds(); // rectangle of game window (x,y always 0,0)

	void SetTimeScale(float scale);
	float GetTimeScale();

	void BeginMagmaDrawing();
	void EndMagmaDrawing();

	void DrawCheckeredBackground(int tileSize, const char* text, Color color, Color altColor, Color highlightColor, Color textColor);
	void DrawBoundingBox2D(BoundingBox2D bounds, Color tint);

	void DrawRetroText(const char* text, int posX, int posY, Color color);
	void DrawRetroTextV(const char* text, Vector2 pos, Color color);
	void DrawRetroTextEx(const char* text, int posX, int posY, int fontSize, Color color);
	void DrawRetroTextPro(const char* text, Vector2 pos, int fontSize, Color color);

	Vector2 MeasureRetroText(const char* text);
	Vector2 MeasureRetroTextEx(const char* text, int fontSize);

	Ray GetWindowMouseRay(Camera3D camera);

	Vector2 GetWindowMousePositionEx(Camera2D camera);
	Vector2 GetWindowMousePosition();
	Vector2 GetWindowTopLeft();
	Vector2 GetWindowCenter();

	// entities
	void TranslateEntityV(EntityID id, Vector2 offset);
	void TranslateEntity(EntityID id, float x, float y);

	void Translate3DEntityV(EntityID id, Vector3 offset);
	void Translate3DEntity(EntityID id, float x, float y, float z);

	void SetEntityCenterV(EntityID id, Vector2 pos);
	void SetEntityCenter(EntityID id, float x, float y);

	void Set3DEntityCenterV(EntityID id, Vector3 pos);
	void Set3DEntityCenter(EntityID id, float x, float y, float z);

	void SetEntitySizeV(EntityID id, Vector2 pos);
	void SetEntitySize(EntityID id, float x, float y);

	void Set3DEntitySizeV(EntityID id, Vector3 pos);
	void Set3DEntitySize(EntityID id, float x, float y, float z);

	void ResetEntityTranslation(EntityID id);

	void ResetEntityTranslation(EntityID id);
	void MagmaLogger(int msgType, const char* text, va_list args);
	void SetTraceLogAssertLevel(TraceLogLevel level);
	void SetTraceLogOpenLevel(TraceLogLevel level);
	void ClearLog();

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
	typedef void (*InputBoxEntered)(char* text);
	bool ShowInputBox(const char* title, InputBoxEntered callback);
	bool ShowInputBoxEx(const char* title, InputBoxEntered callback, const char* defText, uint minLength, uint maxLength);

	bool EditorIsOpen();
	void OpenEditor();
	void CloseEditor();
	bool ToggleEditor();

	// contains more "advanced" components and entities, not needed for some types of games
	// can be removed from engine
	// TODO: Make optional with build flag

	// TODO: Add texture support
	typedef struct {
		Color backColor;
		Color lineColor;
		Color textColor;

		int fontSize;
		float padding;

		float arrowPadding;
		float arrowScale;
		float arrowOscil;
		bool arrowTumbleMode;
	} PopMenuConfig;
	// TODO: get default function

	typedef enum {
		POSE_IDLE,
		POSE_WALK,
		POSE_SLIDE,
		POSE_JUMP,
		POSE_FALL,
		POSE_DUCK,
	} PlayerPose;

	inline const char** GetPlayerPoseNames() {
		const char* names[] = {
			STRING(POSE_IDLE),
			STRING(POSE_WALK),
			STRING(POSE_SLIDE),
			STRING(POSE_JUMP),
			STRING(POSE_FALL),
			STRING(POSE_DUCK),
			NULL
		};
		return names;
	};

	typedef enum {
		PLAY_LOOP,
		PLAY_PING_PONG,
		PLAY_ONCE,
	} PlaybackMode;

#define COMP_PHYS_BODY      10
#define COMP_ANIM_PLAYER    11
#define COMP_PLAT_PLAYER    12

	typedef struct {
		const char* name; // description 
		const char* sheetName; // name of texture
		Vector2 origin; // top left of sector
		Vector2 cellSize; // size of each cell
		uint count; // number of cells in anim
		PlaybackMode mode;
		float fps;
	} SheetAnimation;

	// entity group functions
	void SetEntityGroupCamera2D(Camera2D camera);
	void SetEntityGroupCamera3D(Camera3D camera);

#ifdef __cplusplus
}
#endif