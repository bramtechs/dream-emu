// Based on Tsoding Daily: New Build System in C — Part 1
// https://youtu.be/n47AFxc1ksE

#include <iostream>
#include <assert.h>
#include <errno.h>
#include <vector>
#include <initializer_list>
#include <filesystem>

#if __linux__
#include <sys/wait.h>
#include <unistd.h>
#elif defined(_WIN32) || defined(WIN32)
#include <windows.h>
#include <processthreadsapi.h>
#endif

#include <sys/stat.h>
#include <sys/types.h>

#define PATH_SEP '/'

#include "utils.cpp"

#define CMD_ARG(I) argc > I ? std::string(argv[I]):""

typedef void (*CommandFunc)();
struct Command {
    std::string name;
    std::string desc;
    CommandFunc func;
};

struct Repo {
    std::string folder;
    std::string url;
};

// tools required to build the program
std::vector<std::string> REQUIRED_TOOLS = {
    "cmake",
    "git",
#if __linux__
    "g++",
#elif defined(_WIN32) || defined(WIN32)
    "cl",
#endif
};


// tools required to build the program
std::vector<Repo> REQUIRED_REPOS = {
    { "magma/raylib","https://github.com/raysan5/raylib" },
    { "magma/box2d","https://github.com/tobanteGaming/Box2D-cmake.git" },
};

// check if all repos are downloaded
bool check(){
    for (Repo repo : REQUIRED_REPOS) {
        if (!EXISTS(repo.folder)){
            std::cout < "Library " << repo.folder << " is missing" << std::endl;
            return false;
        }
    }
    std::cout < "Found all libraries" << std::endl;
    return true;
}

void download() {
    std::cout << "Checking if libraries downloaded..." << std::endl;
    for (Repo repo : REQUIRED_REPOS) {
        if (EXISTS(repo.folder)) {
            std::cout << "Updating repo " << repo.folder << std::endl;
            CMD({"git", "fetch", repo.folder, "-p"});
            CMD({"git", "merge", repo.folder});
        }else{
            std::cout << "Dowloading repo " << repo.url << std::endl;
            CMD({"git", "clone", repo.url, repo.folder});
        }
    }
}

void generate() {
    if (!check()){
        download();
    }

    std::cout << "Generating cmake project..." << std::endl;
#if __linux__

#elif defined(_WIN32) || defined(WIN32)
    CMD({"cmake" "-S" "." "-B" "build" "-G" "Visual Studio 17 2022" "-A" "Win32"});
#endif
}

void build() {
    if (!EXISTS("build")){
        generate();
    }
    std::cout << "Building cmake project..." << std::endl;
    CMD({"cmake" "--build" "build"});
}

void run() {
    std::cout << "Running..." << std::endl;
}

// valid commands
std::vector<Command> COMMANDS = {
    { "check", "Check if required programs are installed (TODO)", check },
    { "download", "Clone required libraries from Github", download },
    { "generate", "Generate CMake project files", generate },
    { "build", "Build project", build },
    { "run", "Run executable", run },
};

void main(int argc, char** argv) {
    std::string option = CMD_ARG(1);
    if (!option.empty()) {
        if (option == "gen")
            generate();
        else if (option == "check")
            check();
        else if (option == "download")
            download();
        else if (option == "build")
            build();
        else if (option == "run")
            run();
        else
            std::cout << "Invalid option argument: " << option << std::endl;
    } else {
        std::cout << "No command given! " << std::endl;
    }
}
