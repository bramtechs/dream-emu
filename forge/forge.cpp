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

// tools required to build the program UNIMPLEMENTED
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
bool owns_libs(){
    for (Repo repo : REQUIRED_REPOS) {
        if (!EXISTS(repo.folder) || !SUBFILES_EXISTS(repo.folder)){
            std::cout << "Library " << repo.folder << " is missing" << std::endl;
            return false;
        }
    }
    std::cout << "Found all libraries" << std::endl;
    return true;
}

void check(){
    owns_libs();
}

void wipe(){
    std::cout << "Wiping libraries!" << std::endl;
    for (Repo repo : REQUIRED_REPOS) {
        std::cout << ".. " << repo.folder << std::endl;
        std::filesystem::remove_all(repo.folder);
    }
}

void download() {
    std::cout << "Checking if libraries are present..." << std::endl;
    for (Repo repo : REQUIRED_REPOS) {
        if (EXISTS(repo.folder) && SUBFILES_EXISTS(repo.folder)) {
            std::cout << "Updating repo " << repo.folder << std::endl;
            CMD({"git", "fetch", repo.folder, "-p"});
            CMD({"cd",repo.folder,"&","git", "merge"});
        } else {
            if (EXISTS(repo.folder)){ // failsafe if folder sticked around
                std::cout << "Incomplete repo found! Wiping it..." << std::endl;
                std::filesystem::remove_all(repo.folder);
            }
            std::cout << "Dowloading repo " << repo.url << std::endl;
            CMD({"git", "clone", repo.url, repo.folder, "--recursive"});
        }
    }
}

void generate() {
    if (!owns_libs()){
        download();
    }

    std::cout << "Generating cmake project..." << std::endl;
#if __linux__
    std::cout << "TODO add linux support" << std::endl;
#elif defined(_WIN32) || defined(WIN32)
    CMD({"cmake" "-S" "." "-B" "build" "-G" "Visual Studio 17 2022" "-A" "Win32"});
#endif
}

void build() {
    if (!EXISTS("build")){
        generate();
    }
    if (!owns_libs()){
        download();
    }
    std::cout << "Building cmake project..." << std::endl;
    CMD({"cmake", "--build", "build"});
}

void release() {
    if (!EXISTS("build")){
        generate();
    }
    if (!owns_libs()){
        download();
    }
    std::cout << "Building cmake release project..." << std::endl;
    CMD({"cmake", "--build", "build", "--config", "Release"});
}

void run() {
    std::cout << "Running..." << std::endl;
}

void help();

// valid commands
std::vector<Command> COMMANDS = {
    { "check", "Check if required programs are installed (TODO)", check },
    { "download", "Clone required libraries from Github or merge new commits.", download },
    { "generate", "Generate CMake project files", generate },
    { "build", "Build project (for debugging)", build },
    { "release", "Build and package executable", release },
    { "run", "Run executable", run },
    { "wipe", "Remove all cloned libraries (use if things broke)", wipe },
    { "help", "Show this screen", help },
};

void help(){
    std::cout << "" << std::endl;
    for (const auto& cmd : COMMANDS){
        std::string tabs;
        if (cmd.name.size() > 5){
            tabs = "\t\t";
        }else{
            tabs = "\t\t\t";
        }
        std::cout << cmd.name << tabs << cmd.desc << std::endl;
    }
}

void main(int argc, char** argv) {
    std::string option = CMD_ARG(1);
    if (!option.empty()) {
        for (const auto& cmd : COMMANDS){
            if (cmd.name == option){
                (*cmd.func)();
                return;
            }
        }
        std::cout << "Invalid option argument: " << option << std::endl;
    } else {
        std::cout << "No command given! " << std::endl;
        help();
    }
}
