#pragma once

#include "raylib.h"

#ifdef __unix___

#define INFO(X...) TraceLog(LOG_INFO,X)
#define WARN(X...) TraceLog(LOG_WARNING,X)
#define ERROR(X...) TraceLog(LOG_ERROR,X)

#elif defined(_WIN32) || defined(WIN32)

#define INFO(...) TraceLog(LOG_INFO,__VA_ARGS__)
#define WARN(...) TraceLog(LOG_WARNING,__VA_ARGS__)
#define ERROR(...) TraceLog(LOG_ERROR,__VA_ARGS__)

#endif
