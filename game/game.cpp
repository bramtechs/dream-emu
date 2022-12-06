
#include "raylite.hpp"
#include "shared.hpp"

void logger_log(const char* msg){
    printf("--> %s", msg);
}

void logger_warn(const char* msg){
    printf("--> WARN: %s", msg);
}

void logger_error(const char* msg){
    printf("--> ERROR: %s", msg);
}

#include "blocks.cpp"
#include "levels.cpp"