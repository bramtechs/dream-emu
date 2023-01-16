#include "logger.h"

static LogBuffer Buffer = { 0 };

void LoggerLog(TraceLogLevel level, const char* text){
    Buffer.levels[Buffer.count] = level;

    strcpy(Buffer.lines[Buffer.count],text);

    Buffer.count++;
    if (Buffer.count >= MAX_LINES){
        Buffer.count = 0;
    }
}

void DrawLog(float offsetX, float offsetY, int fontSize){
    for (int i = 0; i < Buffer.count; i++){
        const char* text = Buffer.lines[i];
        int y = i * (fontSize+4);

        Color color;
        switch (Buffer.levels[i]){
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

        DrawText(text,offsetX,offsetY+y,fontSize,color);
    }
}
