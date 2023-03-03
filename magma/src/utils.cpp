#include "magma.h"

#include <filesystem>

bool IS_DEBUG = false;

std::vector<std::string> CommandArguments;
bool InitializedArguments;

void RegisterArguments(int argc, char** argv){
    if (InitializedArguments){
        ERROR("Already registered application arguments.");
    }else{
        for (int i = 0; i < argc; i++){
            CommandArguments.push_back(argv[i]);
        }
        DEBUG("Registered %d application arguments.", argc);

        // interpret given arguments
        if (IsRunningWithArguments({"--debug","--dev","-d"})){
            IS_DEBUG = true;
        }
        if (IsRunningWithArguments({"--console","--verbose","-v"})){
            OpenSystemConsole();
        }
    }
}

bool IsRunningWithArguments(std::initializer_list<std::string> args){
    for (const auto& arg : CommandArguments){
        for (const auto& option : args){
            if (arg == option){
                return true;
            }
        }
    }
    return false;
}

// TODO put logger stuff in struct
struct LogLine {
    TraceLogLevel level;
    char msg[512];
};

// TODO: structify
static std::vector<LogLine> Buffer;
static size_t Allocations = 0;
static bool ShowLogger = false;
static float LoggerOffsetY = 0.f;
static TraceLogLevel AssertLevel = LOG_NONE;
static TraceLogLevel OpenLevel = LOG_NONE;

MovingRectangle::MovingRectangle(Rectangle rect, Vector2 vel) {
    this->vx = vel.x;
    this->vy = vel.y;
    this->x = rect.x;
    this->y = rect.y;
    this->w = rect.width;
    this->h = rect.height;
}

bool CreateDirectory(const char* path) {
    if (DirectoryExists(path)) {
        DEBUG("Directory %s already exists, skipping folder creation...", path);
        return true;
    }

    if (!std::filesystem::create_directory(path)) {
        ERROR("Failed to create directory %s! (code %d)", path);
        return false;
    }
    DEBUG("Made directory %s", path);
    return true;
}

static const char* MsgLevelToStr(int msgType){
    switch (msgType)
    {
        case LOG_INFO: 
            return "INFO";
        case LOG_ERROR:
            return "ERROR";
        case LOG_WARNING:
            return "WARN";
        case LOG_DEBUG:
            return "DEBUG";
        default: 
            return "";
    }

}

void MagmaLogger(int msgType, const char* text, va_list args)
{
    // collect current time
    char timeStr[64] = { 0 };
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", tm_info);

    char buffer[512];
    vsnprintf(buffer, 255, text, args);

    const char* levelText = MsgLevelToStr(msgType);
    const char* msg = TextFormat("[%s] [%s] : %s", timeStr, levelText, buffer);

    std::cout << msg << '\n';

    LogLine line;
    line.level = (TraceLogLevel)msgType;
    memcpy(line.msg, msg, 512);
    Buffer.push_back(line);

    // open logger on error
    if (msgType >= OpenLevel) {
        ShowLogger = true;
    }

    if (msgType == AssertLevel) {
        assert(false);
    }
}

void SetTraceLogAssertLevel(TraceLogLevel level) {
    AssertLevel = level;
}

void SetTraceLogOpenLevel(TraceLogLevel level) {
    OpenLevel = level;
}

void ClearLog() {
    Buffer.clear();
}

void DrawLog(float offsetX, float offsetY, int fontSize, bool drawBG) {
    static int bgWidth = 0;

    // draw background
    if (drawBG) {
        Color bgCol = ColorAlpha(BLACK, 0.6f);
        DrawRectangle(0, 0, bgWidth + 25, GetScreenHeight(), bgCol);

        // draw tooltip
        DrawRetroText("Show/hide logs with F2\nScroll by holding LEFT SHIFT.", 10, GetScreenHeight() - 40, 16, WHITE);
    }

    // draw lines
    bgWidth = 0;
    for (int i = 0; i < Buffer.size(); i++) {
        auto& line = Buffer.at(Buffer.size() - i - 1);
        int y = i * (fontSize + 4);

        Color color;
        switch (line.level) {
        case LOG_WARNING:
            color = YELLOW;
            break;
        case LOG_ERROR:
            color = RED;
            break;
        case LOG_DEBUG:
            color = GRAY;
            break;
        default:
            color = WHITE;
            break;
        }

        const char* text = line.msg;
        DrawRetroText(text, offsetX, offsetY + y + LoggerOffsetY, fontSize, color);

        // expand background if needed
        if (drawBG) {
            int len = MeasureText(text, fontSize);
            if (len > bgWidth) {
                bgWidth = len;
            }
        }
    }
}

void UpdateAndDrawLog(float offsetX, float offsetY, int fontSize) {
    if (IsKeyPressed(KEY_F2)) {
        ToggleLogger();
        LoggerOffsetY = 0.f;
    }
    if (ShowLogger) {
        DrawLog(offsetX, offsetY, fontSize);

        // scrolling
        if (IsKeyDown(KEY_LEFT_SHIFT)) {
            LoggerOffsetY += GetMouseWheelMove() * 100;
        }
    }
}

bool LoggerIsOpen() {
    return ShowLogger;
}

void OpenLogger() {
    ShowLogger = true;
}

void CloseLogger() {
    ShowLogger = false;
}

bool ToggleLogger() {
    ShowLogger = !ShowLogger;
    return ShowLogger;
}

void* M_MemAlloc(size_t size) {
    void* ptr = MemAlloc(size);
    Allocations++;
    return ptr;
}

void M_MemFree(void* ptr)
{
    MemFree(ptr);
    Allocations--;
}

void CheckAllocations() {
    if (Allocations == 0) {
        INFO("All allocations got freed!");
    }
    else {
        WARN("%d allocations did not get freed!", Allocations);
    }
}

Vector3 Vector2ToVector3(Vector2 vec2){
    return {
        vec2.x,
        vec2.y,
        0.f
    };
}

Vector2 Vector3ToVector2(Vector3 vec3){
    return {
        vec3.x,
        vec3.y,
    };
}

Vector2 Vector2Absolute(Vector2 v2) {
    return {
        fabs(v2.x),
        fabs(v2.y),
    };
}

Vector2 Vector2Snap(Vector2 v2, float gridSize) {
    Vector2 cell = {
        (int)(v2.x / gridSize) * (float)gridSize,
        (int)(v2.y / gridSize) * (float)gridSize,
    };
    cell.x -= cell.x < 0 ? gridSize : 0.f;
    cell.y -= cell.y < 0 ? gridSize : 0.f;

    return cell;
}

Vector3 Vector3Absolute(Vector3 v3) {
    return {
        fabs(v3.x),
        fabs(v3.y),
        fabs(v3.z),
    };
}

Vector3 Vector3Snap(Vector3 v3, float gridSize) {
    Vector3 cell = {
        (int)(v3.x / gridSize) * (float)gridSize,
        (int)(v3.y / gridSize) * (float)gridSize,
        (int)(v3.z / gridSize) * (float)gridSize,
    };
    cell.x -= cell.x < 0 ? gridSize : 0.f;
    cell.y -= cell.y < 0 ? gridSize : 0.f;
    cell.z -= cell.z < 0 ? gridSize : 0.f;

    return cell;
}

Color InvertColor(Color col, bool invertAlpha) {
    col.r = 255 - col.r;
    col.g = 255 - col.g;
    col.b = 255 - col.b;
    if (invertAlpha) {
        col.a = 255 - col.a;
    }
    return col;
}

Color ColorLerp(Color src, Color dst, float factor) {
    factor = Clamp(factor, 0.f, 1.f);

    src.r = Lerp(src.r, dst.r, factor);
    src.g = Lerp(src.g, dst.g, factor);
    src.b = Lerp(src.b, dst.b, factor);
    src.a = Lerp(src.a, dst.a, factor);

    return src;
}

// c^2 = a^2 + b^2
float GetRectangleDiameter(Rectangle rec) {
    float diam2 = GetRectangleDiameterSquared(rec);
    return sqrtf(diam2);
}
float GetRectangleDiameterSquared(Rectangle rec) {
    return rec.width * rec.width + rec.height * rec.height;
}

float SweptAABB(MovingRectangle source, MovingRectangle target, Vector2* normal) {
    MovingRectangle& b1 = source;
    MovingRectangle& b2 = target;

    float xInvEntry, yInvEntry;
    float xInvExit, yInvExit;

    // find the distance between the objects on the near and far sides for both x and y 
    if (b1.vx > 0.0f)
    {
        xInvEntry = b2.x - (b1.x + b1.w);
        xInvExit = (b2.x + b2.w) - b1.x;
    }
    else
    {
        xInvEntry = (b2.x + b2.w) - b1.x;
        xInvExit = b2.x - (b1.x + b1.w);
    }

    if (b1.vy > 0.0f)
    {
        yInvEntry = b2.y - (b1.y + b1.h);
        yInvExit = (b2.y + b2.h) - b1.y;
    }
    else
    {
        yInvEntry = (b2.y + b2.h) - b1.y;
        yInvExit = b2.y - (b1.y + b1.h);
    }

    // find time of collision and time of leaving for each axis (if statement is to prevent divide by zero) 
    float xEntry, yEntry;
    float xExit, yExit;

    if (b1.vx == 0.0f)
    {
        xEntry = -std::numeric_limits<float>::infinity();
        xExit = std::numeric_limits<float>::infinity();
    }
    else
    {
        xEntry = xInvEntry / b1.vx;
        xExit = xInvExit / b1.vx;
    }

    if (b1.vy == 0.0f)
    {
        yEntry = -std::numeric_limits<float>::infinity();
        yExit = std::numeric_limits<float>::infinity();
    }
    else
    {
        yEntry = yInvEntry / b1.vy;
        yExit = yInvExit / b1.vy;
    }

    // find the earliest/latest times of collisionfloat 
    float entryTime = std::max(xEntry, yEntry);
    float exitTime = std::min(xExit, yExit);


    // if there was no collision
    if (entryTime > exitTime || xEntry < 0.0f && yEntry < 0.0f || xEntry > 1.0f || yEntry > 1.0f)
    {
        normal->x = 0.0f;
        normal->y = 0.0f;
        return 1.0f;
    }
    else // if there was a collision 
    {
        // calculate normal of collided surface
        if (xEntry > yEntry)
        {
            if (xInvEntry < 0.0f)
            {
                normal->x = 1.0f;
                normal->y = 0.0f;
            }
            else
            {
                normal->x = -1.0f;
                normal->y = 0.0f;
            }
        }
        else
        {
            if (yInvEntry < 0.0f)
            {
                normal->x = 0.0f;
                normal->y = 1.0f;
            }
            else
            {
                normal->x = 0.0f;
                normal->y = -1.0f;
            }
        } // return the time of collision
        return entryTime;
    }
}

Rectangle BoundingBoxToRect(BoundingBox box) {
    Vector2 pos = { box.min.x, box.min.y };
    Vector3 size3 = Vector3Subtract(box.max, box.min);
    Vector2 size = { size3.x, size3.y };
    return { pos.x, pos.y, size.x, size.y };
}

Rectangle BoundingBoxToRect(BoundingBox2D box) {
    Vector2 pos = box.min;
    Vector2 size = Vector2Subtract(box.max, box.min);
    return { pos.x, pos.y, size.x, size.y };
}

std::string GetTempDirectory() {
    std::filesystem::path path = std::filesystem::temp_directory_path();
    std::string tempFolStr = path.string();
    return tempFolStr;
}
