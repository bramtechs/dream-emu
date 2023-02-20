// Based on Tsoding Daily: New Build System in C — Part 1
// https://youtu.be/n47AFxc1ksE

#include <iostream>
#include <vector>

#if __linux__
    #define LINUX
#elif defined(_WIN32) || defined(WIN32)
    #define WINDOWS
#endif

// =============================
// === PROJECT CONFIGURATION ===
// =============================

// set to true if configured properly
constexpr bool IS_CONFIGURED = true;

// UNIMPLEMENTED: tools required to build the program
std::vector<std::string> REQUIRED_TOOLS = {
    "cmake",
    "git",
#ifdef LINUX
    "g++",
#elif defined(WINDOWS)
    "cl",
#endif
};

// online libraries required to build the program
std::vector<std::pair<std::string,std::string>> REQUIRED_REPOS = {
    { "magma/raylib","https://github.com/raysan5/raylib" },
    { "magma/box2d","https://github.com/tobanteGaming/Box2D-cmake.git" },
};

// =============================

#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <initializer_list>
#include <filesystem>
#include <sstream>
#include <thread>

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

bool subfiles_exist(std::string path) {
    int count = 0;
    for (const auto& file : std::filesystem::directory_iterator(path)){
        if (!file.is_directory()){
            count++;
        }
    }
    return count > 0;
}

std::string concat_sep(char sep, std::initializer_list<std::string> list) {
    std::string result = "";
    for (const auto elem : list) {
        result += elem;
        result += sep;
    }
    return result;
}

bool make_dirs(std::initializer_list<std::string> list){
    std::string path = concat_sep(PATH_SEP,list);
    try {
        std::filesystem::create_directories(path);
        return true;
    } catch(std::filesystem::filesystem_error const& ex){
        std::cerr << "Failed to create directories!" << std::endl;
        std::cerr << ex.what() << std::endl;
        return false;
    }
}

// TODO: don't crash on linux when cmd fails
bool run_command(std::initializer_list<std::string> list){
#if __linux__
    size_t argc = list.count;
    const char **argv = malloc(sizeof(const char*) * (argc - 1));

    argc = 0;
    printf(">> ");
    va_start(args,silent);
    for (const auto elem : list){
        argv[argc++] = arg;
        printf("%s ",arg);
    }
    printf("\n");

    assert(argc >= 1);
    int cpid = fork();
    if (cpid == -1){
        std::cerr << "Could not fork a child process! " << strerror(errno)
                                                        << std::endl;
        return false;
    }

    if (cpid == 0){
        if (execvp(argv[0], (char * const*) argv) < 0){
            std::cerr << "Sub-process failed: " << strerror(errno) << std::endl;
            return false;
        }
    }
    else if (cpid > 0) {
        wait(NULL); // wait for child to end
    }
    else {
        std::cerr << ("Fork failed: " << strerror(errno) << std::endl;
        return false;
    }

#elif defined(_WIN32) || defined(WIN32)

    if (!system(NULL)){
        std::cerr << "No command processor found!" << std::endl;
        return false;
    }

    std::string cmd = concat_sep(' ',list);
    std::cout << ">> " << cmd << std::endl;

    int code = system(cmd.c_str());
    return code == 0;
#endif
}

bool delete_recursive(std::string path){
    try {
        std::filesystem::remove_all(path);
        return true;
    } catch (std::filesystem::filesystem_error const& ex){
        std::cerr << "Failed to remove directories!" << std::endl;
        std::cerr << ex.what() << std::endl;
        return false;
    }
}

bool check(){
    std::cout << "UNIMPLEMENTED!" << std::endl;
    return true;
}

bool clean(){
    if (std::filesystem::exists("build")){
        return delete_recursive("build");
    }
    return true;
}

bool wipe(){
    std::cout << "Wiping libraries!" << std::endl;
    for (const auto& repo : REQUIRED_REPOS) {
        std::cout << ".. " << repo.first << std::endl;
        if (!delete_recursive(repo.first)){
            return false;
        }
    }
    return true;
}

bool download() {
    // download libraries
    std::cout << "Checking if libraries are present..." << std::endl;
    for (const auto& repo : REQUIRED_REPOS) {
        if (std::filesystem::exists(repo.first) && subfiles_exist(repo.first)) {
            std::cout << "Updating repo " << repo.first << std::endl;

            if (!run_command({"git", "fetch", repo.first, "-p"})){
                return false;
            }
            if (!run_command({"cd",repo.first,"&","git", "merge"})){
                return false;
            }

        } else {

            // fail-safe if folder sticked around
            if (std::filesystem::exists(repo.first)){ 
                std::cout << "Incomplete repo found! Wiping it..." << std::endl;
                if (!delete_recursive(repo.first)){
                    return false;
                }
            }

            std::cout << "Dowloading repo " << repo.second << std::endl;
            if (!run_command({"git", "clone", repo.second, repo.first, "--recursive"})){
                return false;
            }
        }
    }
    return true;
}

bool generate() {
    if (!download()){
        return false;
    }

    std::cout << "Generating cmake project..." << std::endl;
#if LINUX
    std::cout << "TODO add linux support" << std::endl;
    return false;
#elif defined(WINDOWS)
    return run_command({"cmake", "-S", ".", "-B", "build", "-G", "\"Visual Studio 17 2022\"", "-A", "Win32"});
#endif
}

bool build() {
    if (!std::filesystem::exists("build")){
        if (!generate()){
            return false;
        }
    }

    unsigned int threads = std::thread::hardware_concurrency();
    std::cout << "Building cmake project..." << std::endl;
    if (threads > 0){
        return run_command({"cmake", "--build", "build", "-j", std::to_string(threads)});
    }
    return run_command({"cmake", "--build", "build"});
}

bool release() {
    if (!std::filesystem::exists("build")){
        if (!generate()){
            return false;
        }
    }

    unsigned int threads = std::thread::hardware_concurrency();
    std::cout << "Building cmake project..." << std::endl;
    if (threads > 0){
        return run_command({"cmake", "--build", "build", "-j", std::to_string(threads), "--config", "Release"});
    }
    return run_command({"cmake", "--build", "build", "--config", "Release"});
}

bool package(){
    if (!release()){
        return false;
    }

    return false;
}

bool run() {
    std::cout << "Running..." << std::endl;
    return false;
}

// === main cli functions

typedef bool (*CommandFunc)();
struct Command {
    std::string name;
    std::string desc;
    CommandFunc func;
};

// valid commands
bool help();
std::vector<Command> COMMANDS = {
    { "check", "Check if required programs are installed (TODO)", check },
    { "download", "Clone required libraries from Github or merge new commits.", download },
    { "generate", "Generate CMake project files", generate },
    { "build", "Build project (for debugging)", build },
    { "release", "Build optimized executable", release },
    { "package", "Build and package optimized executable", package },
    { "run", "Run executable", run },
    { "wipe", "Remove all cloned libraries (use if things broke)", wipe },
    { "clean", "Remove build folder", clean },
    { "help", "Show this screen", help },
};

bool help(){
    // calculate longest width first
    int width = 0;
    for (const auto& cmd : COMMANDS){
        int len = cmd.name.length();
        if (len > width){
            width = len;
        }
    }
    width += 3;

    std::cout << std::endl;
    for (const auto& cmd : COMMANDS){

        // pad string
        std::stringstream displayName;
        displayName << cmd.name;

        int spaces = width - cmd.name.length();
        for (int i = 0; i < spaces; i++){
            displayName << " ";
        }

        std::cout << displayName.str() << cmd.desc << std::endl;
    }
    return true;
}

bool run_option(std::string& option){
    if (option.empty()){
        std::cout << "No command given! " << std::endl;
        return false;
    }

    for (const auto& cmd : COMMANDS){
        if (cmd.name == option) {
            bool succeeded = (*cmd.func)();
            if (succeeded) {
                std::cout << "Done...";
                return true;
            }
            std::cerr << "Failed.";
            return false;
        }
    }

    // invalid option
    std::cout << "Invalid option: " << option << std::endl;
    help();
    return false;
}

bool relocate(){
    try {
        auto path = std::filesystem::current_path();
        auto parentDir = path.parent_path();
        std::filesystem::current_path(parentDir);
        return true;
    } catch(std::exception const& ex){
        std::cerr << "Failed to relocate working directory!" << std::endl;
        return false;
    }
}

bool find_cmake_project(){
    if (!std::filesystem::exists("CMakeLists.txt")){
        std::cout << "Execution directory does not contain CMakeLists.txt!" << std::endl;
        std::cout << "Looking in parent directory..." << std::endl;
        if (relocate()){
            if (std::filesystem::exists("CMakeLists.txt")){
                std::cout << "Found CMakeLists!" << std::endl;
                return true;
            }else{
                std::cout << "Didn't find CMakeLists.txt!" << std::endl;
            }
        }
        return false;
    }
    std::cout << "Found CMakeLists!" << std::endl;
    return true;
}

int main(int argc, char** argv) {
    std::string option = argc > 1 ? std::string(argv[1]):"";
    if (option == "help"){
        help();
        return EXIT_SUCCESS;
    }

    // check if cmake file found
    if (!find_cmake_project()){
        return EXIT_FAILURE;
    }

    if (IS_CONFIGURED){
        if (run_option(option)){
            return EXIT_SUCCESS;
        }
    }else{
        std::cout << "Edit forge.cpp first and configure the program!" << std::endl;
    }

    return EXIT_FAILURE;
}
