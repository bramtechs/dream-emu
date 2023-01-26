#include "magma.h"

static std::vector<LogLine> Buffer;
static size_t Allocations = 0;
static bool ShowLogger = false;
static float LoggerOffsetY = 0.f;

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

void MagmaLogger(int msgType, const char* text, va_list args)
{
    char timeStr[64] = { 0 };
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);

    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", tm_info);
    printf("[%s] ", timeStr);

    switch (msgType)
    {
    case LOG_INFO: printf("[INFO] : "); break;
    case LOG_ERROR: printf("[ERROR]: "); break;
    case LOG_WARNING: printf("[WARN] : "); break;
    case LOG_DEBUG: printf("[DEBUG]: "); break;
    default: break;
    }

    char buffer[512];
    vsnprintf(buffer, 255, text, args);

    std::cout << buffer << std::endl;

    LogLine line = {
        (TraceLogLevel)msgType,
        buffer
    };
    Buffer.push_back(line);

    // open logger on error
    if (msgType == LOG_ERROR || msgType == LOG_WARNING ) {
        ShowLogger = true;
    }
}

void ClearLog() {
    Buffer.clear();
}

void DrawLog(float offsetX, float offsetY, int fontSize) {
    for (int i = 0; i < Buffer.size(); i++) {
        auto &line = Buffer.at(Buffer.size()-i-1);
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

        const char* text = line.text.c_str();
        DrawText(text, offsetX, offsetY + y + LoggerOffsetY, fontSize, color);
    }
}

void UpdateAndDrawLog(float offsetX, float offsetY, int fontSize) {
    if (IsKeyPressed(KEY_F2)) {
        ShowLogger = !ShowLogger;
        LoggerOffsetY = 0.f;
    }
    if (ShowLogger) {
        DrawLog(offsetX, offsetY, fontSize);

        // scrolling
        LoggerOffsetY += GetMouseWheelMove() * 100;
    }
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

Color InvertColor(Color col, bool invertAlpha) {
    col.r = 255 - col.r;
    col.g = 255 - col.g;
    col.b = 255 - col.b;
    if (invertAlpha) {
        col.a = 255 - col.a;
    }
    return col;
}

Color ColorLerp(Color src, Color dst, float factor){
    factor = Clamp(factor,0.f,1.f);

    src.r = Lerp(src.r,dst.r,factor);
    src.g = Lerp(src.g,dst.g,factor);
    src.b = Lerp(src.b,dst.b,factor);
    src.a = Lerp(src.a,dst.a,factor);
    
    return src;
}
