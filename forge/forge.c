// Based on Tsoding Daily: New Build System in C — Part 1
// https://youtu.be/n47AFxc1ksE

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <errno.h>

#if __linux__
#include <unistd.h>
#include <sys/wait.h>
#elif defined(_WIN32) || defined(WIN32)
#include <windows.h>
#include <processthreadsapi.h>
#endif

#include <sys/stat.h>
#include <sys/types.h>

#define PATH_SEP "/"
#define PATH_SEP_LEN (sizeof(PATH_SEP)-1)

#define PRINT(...)  printf(__VA_ARGS__); printf("\n")
#define PRINT_ERR(...)  fprintf(stderr,"[ERROR]: "); \
                    fprintf(stderr,__VA_ARGS__); \
                    printf("\n") \

#include "utils.c"

#define PATH(...)   form_path(69,__VA_ARGS__,NULL)
#define MKDIRS(...) make_dirs(420,__VA_ARGS__,NULL)
#define CMD(...)    run_cmd(false,__VA_ARGS__,NULL)
#define CMD_SILENT(...)    run_cmd(true,__VA_ARGS__,NULL)

#define CMD_ARG(I) argc > I ? argv[I]:NULL

typedef void (*CommandFunc)();
typedef struct {
    const char* name;
    const char* desc;
    CommandFunc func;
} Command;

// tools required to build the program
const char* REQUIRED_TOOLS[] = {
    "cmake",
    "git",
#if __linux__
    "g++",
#elif defined(_WIN32) || defined(WIN32)
    "cl",
#endif
    NULL, // don't forget to end the array
};

// check if all tools are installed
void check(){
    PRINT("Checking required tools...");
    for (size_t i = 0; i < 128; i++){
        const char* tool = REQUIRED_TOOLS[i];

        // reached the end of array
        if (tool == NULL)
            break;

        // print result
        bool found = CMD_SILENT(tool);
        PRINT("%s - %s",tool,found ? "FOUND":"MISSING!");
    }
}

void download() {
    PRINT("Checking if libraries downloaded...");
}

void generate() {
    PRINT("Generating cmake project...");
}

void build() {
    PRINT("Building cmake project...");
}

void run() {
    PRINT("Running...");
}

// valid commands
const Command COMMANDS[] = {
    { "check", "Check if required program is installed", check },
    { "download", "Clone required libraries from Github", download },
    { "generate", "Generate CMake project files", generate },
    { "build", "", build },
    { "run", "", run },
    { NULL } // end of array (do not remove)
};

void main(int argc, char** argv) {
    char* option = CMD_ARG(1);
    if (option != NULL) {
        if (strcmp(option, "gen") == 0)
            generate();
        else if (strcmp(option, "check") == 0)
            check();
        else if (strcmp(option, "download") == 0)
            download();
        else if (strcmp(option, "build") == 0)
            build();
        else if (strcmp(option, "run") == 0)
            run();
        else
            PRINT("Invalid option argument: %s",option);
    } else {
        PRINT("No command given!",option);
    }
}
