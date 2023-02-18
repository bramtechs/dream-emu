// Based on Tsoding Daily: New Build System in C — Part 1
// https://youtu.be/n47AFxc1ksE

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#define PATH_SEP "/"
#define PATH_SEP_LEN (sizeof(PATH_SEP)-1)

#define PRINT(...)  printf(__VA_ARGS__); printf("\n")
#define ERROR(...)  fprintf(stderr,"[ERROR]: "); \
                    fprintf(stderr,__VA_ARGS__); \
                    printf("\n") \

#include "utils.c"

#define PATH(...)   form_path(69,__VA_ARGS__,NULL)
#define MKDIRS(...) make_dirs(420,__VA_ARGS__,NULL)
#define CMD(...)    run_cmd(666,__VA_ARGS__,NULL)

#define CMD_ARG(I) argc > I ? argv[I]:NULL

bool check_dependencies(){
    return false;
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

void main(int argc, char** argv) {
    char* option = CMD_ARG(1);
    if (check_dependencies() && option){
        if (strcmp(option, "gen") == 0)
            generate();
        else if (strcmp(option, "download") == 0)
            download();
        else if (strcmp(option, "build") == 0)
            build();
        else if (strcmp(option, "run") == 0)
            run();
        else
            PRINT("Invalid option argument: %s",option);
    }
    else {
        PRINT("Failed");
    }
    CMD("pwd");
}
