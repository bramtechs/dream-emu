#include "magma.h"

static std::vector<LogLine> Buffer;
static size_t Allocations = 0;

bool CreateDirectory(const char* path){
    if (DirectoryExists(path)){
        DEBUG("Directory %s already exists, skipping folder creation...",path);
        return true;
    }

    if (!std::filesystem::create_directory(path)){
        ERROR("Failed to create directory %s! (code %d)", path);
        return false;
    }
    DEBUG("Made directory %s",path);
    return true;
}

void LoggerLog(TraceLogLevel level, const char* text){
    LogLine line = {
        level,
        text
    };
    Buffer.push_back(line);
}

void DrawLog(float offsetX, float offsetY, int fontSize){
    for (int i = 0; i < Buffer.size(); i++) {
        auto line = Buffer.at(i);
        int y = i * (fontSize+4);

        Color color;
        switch (line.level){
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
        DrawText(text, offsetX, offsetY + y, fontSize, color);
    }
}

void* M_MemAlloc(size_t size){
    void* ptr = MemAlloc(size);
    Allocations++;
    return ptr;
}

void M_MemFree(void* ptr)
{
    MemFree(ptr);
    Allocations--;
}

void CheckAllocations(){
    if (Allocations == 0){
        INFO("All allocations got freed!");
    }else{
        WARN("%d allocations did not get freed!",Allocations);
    }
}
