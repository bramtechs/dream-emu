#pragma once

#include "raylib.h"
#include "memory.h"

#define MAX_LINES 128
#define MAX_LINE_LEN 512

typedef struct {
    TraceLogLevel levels[MAX_LINES];
    const char lines[MAX_LINES][MAX_LINE_LEN];
    size_t count;
} LogBuffer;

// do not use these methods directly
void LoggerLog(TraceLogLevel level, const char* name);

void DrawLog(float offsetX, float offsetY, int fontSize);

// use these macro's instead!
#ifdef __unix___

#define INFO(X...)  LoggerLog(LOG_INFO,TextFormat(X));      TraceLog(LOG_INFO,X)
#define WARN(X...)  LoggerLog(LOG_WARNING,TextFormat(X));   TraceLog(LOG_WARNING,X)
#define DEBUG(X...) LoggerLog(LOG_DEBUG,TextFormat(X));     TraceLog(LOG_DEBUG,X)
#define ERROR(X...) LoggerLog(LOG_ERROR,TextFormat(X));     TraceLog(LOG_ERROR,X)

#elif defined(_WIN32) || defined(WIN32)

#define INFO(...)  LoggerLog(LOG_INFO,TextFormat(__VA_ARGS__));    TraceLog(LOG_INFO,__VA_ARGS__)
#define WARN(...)  LoggerLog(LOG_WARNING,TextFormat(__VA_ARGS__)); TraceLog(LOG_WARNING,__VA_ARGS__)
#define DEBUG(...) LoggerLog(LOG_DEBUG,TextFormat(__VA_ARGS__));   TraceLog(LOG_DEBUG,__VA_ARGS__)
#define ERROR(...) LoggerLog(LOG_ERROR,TextFormat(__VA_ARGS__));   TraceLog(LOG_ERROR,__VA_ARGS__)

#endif
