#include <iostream>
#include <vector>
#include <unordered_map>

#ifdef __linux__
    #define LINUX
#elif defined(_WIN32) || defined(WIN32)
    #define WINDOWS
#endif

// =============================
// === PROJECT CONFIGURATION ===
// =============================

std::string RELEASES_DIRECTORY = "releases";
std::string BUILD_DIRECTORY    = "build";

// executable file
#ifdef LINUX

std::string CMAKE_GENERATOR    = "CodeBlocks - Unix Makefiles";
std::string EXECUTABLE_DEBUG   = "build/temple/temple";
std::string EXECUTABLE_RELEASE = EXECUTABLE_DEBUG;

#elif defined(WINDOWS)

std::string CMAKE_GENERATOR    = "Visual Studio 17 2022";
std::string EXECUTABLE_DEBUG   = "build/temple/Debug/temple.exe";
std::string EXECUTABLE_RELEASE = "build/temple/Release/temple.exe";
constexpr bool COMPILE_32BIT   = true;

#endif

// additional files/folders that should be packaged in zip (package command)
std::vector<std::string> ADDITIONAL_FILES = {
    "temple/assets.mga"
};

// Necessary tools to build the program (searches in PATH)
const std::vector<std::string> REQUIRED_TOOLS = {
    "cmake",
    "git",
//#ifdef LINUX
//    "g++",
//#elif defined(WINDOWS)
//    "cl",
//#endif
};

// Online libraries required to build the program (folder + url)
std::vector<std::pair<std::string,std::string>> REQUIRED_REPOS = {
    { "magma/raylib","https://github.com/bramtechs/raylib-lite.git" },
    { "magma/box2d","https://github.com/tobanteGaming/Box2D-cmake.git" },
};

// =============================

#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <initializer_list>
#include <filesystem>
#include <sstream>
#include <fstream>
#include <thread>

#ifdef __linux__
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#elif defined(_WIN32) || defined(WIN32)
#include <windows.h>
#include <processthreadsapi.h>
#endif

#include <sys/stat.h>
#include <sys/types.h>

namespace fs = std::filesystem;

typedef std::unordered_map<std::string,std::string> Table;

const wchar_t PATH_SEP = fs::path::preferred_separator;

// utility functions
static bool subfiles_exist(std::string path);
static bool make_dirs(std::string path);
static std::string concat_sep(char sep, std::initializer_list<std::string> list);
static std::string concat_sep(char sep, std::vector<std::string>& vec);
static bool run_command(std::initializer_list<std::string> list);
static bool run_command(std::vector<std::string> vec);
static bool delete_recursive(std::string path);
static void print_table(Table table, int padding=3);
static bool has_program(std::string program);
static bool make_archive(std::string output, std::vector<std::string>& files);
static std::string get_date_string();

bool check(bool verbose=false){
    bool good = true;
    Table table;
    for (const auto& tool : REQUIRED_TOOLS){
        std::string desc;
        if (has_program(tool)){
            desc = "FOUND";
        }else{
            desc = "MISSING";
            good = false;
        }
        table.insert({tool,desc});
    }
    if (!good || verbose){
        print_table(table);
    }
    if (!good){
        std::cout << "You are missing required software!" << std::endl;
    }
    return good;
}

bool check_verbose(){
    return check(true);
}

bool clean(){
    if (fs::exists(BUILD_DIRECTORY)){
        return delete_recursive(BUILD_DIRECTORY);
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
    std::cout << "Wiping build folder!" << std::endl;
    delete_recursive(BUILD_DIRECTORY);
    return true;
}

bool download() {
    // download libraries
    std::cout << "Checking if libraries are present..." << std::endl;
    for (const auto& repo : REQUIRED_REPOS) {
        if (fs::exists(repo.first) && subfiles_exist(repo.first)) {
            std::cout << "Updating repo " << repo.first << std::endl;

            if (!run_command({"git", "fetch", repo.first, "-p"})){
                return false;
            }
            if (!run_command({"cd",repo.first,"&","git", "merge"})){
                return false;
            }

        } else {

            // fail-safe if folder sticked around
            if (fs::exists(repo.first)){ 
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

// TODO: this is awful
void insert_if_missing(std::string* content, const std::string& text){
    std::stringstream stream(*content);
    std::string line;
    while (std::getline(stream,line)){
        if (line == text){
            // doesn't need to be added
            return;
        }
    }
    // add text to stream
    *content = stream.str() + '\n' + text;
    std::cout << "Added " << *content << " to .gitignore" << std::endl;
}

#ifdef CHECK_GITIGNORE
void check_gitignore() {
    if (fs::exists(".gitignore")){
        std::cout << "Patching .gitignore..." << std::endl;

        // read the file
        std::ifstream ignoreFile(".gitignore");
        std::string content;
        std::string line;
        while (std::getline(ignoreFile, line)) {
            content += line + '\n';
        }
        ignoreFile.close();


        // process file text 
        insert_if_missing(&content, BUILD_DIRECTORY);
        insert_if_missing(&content, RELEASES_DIRECTORY);

        std::cout << content << std::endl;

        // write back into file
        std::ofstream ignoreFileOut(".gitignore");
        ignoreFileOut << content;
        ignoreFileOut.close();
    }
}
#endif

bool generate() {
#ifdef CHECK_GITIGNORE
    check_gitignore();
#endif

    if (!download()){
        return false;
    }

    std::cout << "Generating cmake project..." << std::endl;
    std::string genFull = "\"" + CMAKE_GENERATOR + "\"";
#ifdef LINUX
    return run_command({"cmake", "-S", ".", "-B", "build", "-G", genFull });
#elif defined(WINDOWS)
    if (COMPILE_32BIT){
        return run_command({"cmake", "-S", ".", "-B", "build", "-G", genFull, "-A", "Win32"});
    }
    return run_command({"cmake", "-S", ".", "-B", "build", "-G", genFull});
#endif
}

bool build() {
    if (!fs::exists(BUILD_DIRECTORY)){
        if (!generate()){
            return false;
        }
    }

    unsigned int threads = std::thread::hardware_concurrency();
    std::cout << "Building cmake project..." << std::endl;
    if (threads > 0){
        return run_command({"cmake", "--build", "build", "-j", std::to_string(threads), "--config", "Debug"});
    }
    return run_command({"cmake", "--build", "build", "--config", "Debug"});
}

bool release() {
    if (!fs::exists(BUILD_DIRECTORY)){
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

bool assets() {
#ifdef LINUX
    std::string DEFLATION_PATH = fs::absolute(fs::path(BUILD_DIRECTORY+"/magma/tools/deflation/deflation")).string();
#elif defined(WINDOWS)
    std::string DEFLATION_PATH = fs::absolute(fs::path(BUILD_DIRECTORY+"/magma/tools/deflation/Debug/deflation.exe")).string();
#endif

    if (!fs::exists(DEFLATION_PATH)){
        if (!build()){
            return false;
        }
    }

    std::string outFile = fs::path("temple/assets.mga").string();
    return run_command({ DEFLATION_PATH, "temple/raw_assets", "core_assets", outFile, "--compress"});
}

bool package(){
    std::cout << "Compiling and packaging release build!" << std::endl;

    if (!release()){
        return false;
    }

    if (!make_dirs(RELEASES_DIRECTORY)){
        return false;
    }

    // regenerate assets also
    assets();

    // list files to be archived
    std::vector<std::string> files;
    files.push_back(EXECUTABLE_RELEASE);
    for (const auto& file : ADDITIONAL_FILES){
        files.push_back(file);
    }

    // generate fitting name for package
    auto path = fs::current_path().filename().string();
    std::string pkgName = path + '_' + get_date_string();

    // create the archive
    std::string savePath = concat_sep(PATH_SEP,{RELEASES_DIRECTORY,pkgName});
    if (make_archive(savePath,files)){
        std::cout << "Wrote release archive" << std::endl;
    } else {
        std::cerr << "Failed to write release archive!" << std::endl;
        return false;
    }

    return true;
}

bool run(std::string path) {
    auto p = fs::path(path.c_str());

    // regenerate assets
    assets();

    if (fs::exists(path)) {
#ifdef LINUX
        return run_command({path});
#elif defined(WINDOWS)
        // TODO: will not find assets!
        return run_command({"cd", folder, "&", file });
#endif
    }

    std::cerr << "Executable " << path << " does not exist!" << std::endl;
    return false;
}

bool rundebug() {
    if (!build()){
        return false;
    }
    return run(EXECUTABLE_DEBUG);
}

bool runrel() {
    if (!release()){
        return false;
    }
    return run(EXECUTABLE_RELEASE);
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
    { "check", "Check if required programs are installed", check_verbose },
    { "download", "Clone required libraries from Github or merge new commits.", download },
    { "gen", "Generate CMake project files", generate },
    { "generate", "Generate CMake project files (alias)", generate },
    { "build", "Build project (for debugging)", build },
    { "release", "Build optimized executable", release },
    { "assets", "Repackage game assets", assets },
    { "package", "Build and package optimized executable", package },
    { "run", "Run executable (debug)", rundebug },
    { "runrel", "Run executable (release)", runrel },
    { "clean", "Remove build folder", clean },
    { "wipe", "clean + remove downloaded libraries", wipe },
    { "help", "Show this screen", help },
};

bool help(){
    Table table;
    for (const auto& cmd : COMMANDS){
        table.insert({cmd.name,cmd.desc});
    }
    print_table(table);
    return true;
}

bool run_option(std::string& option){
    if (option.empty()){
        std::cout << "No command given! " << std::endl;
        help();
        return false;
    }

    for (const auto& cmd : COMMANDS){
        if (cmd.name == option) {
            bool succeeded = (*cmd.func)();
            if (succeeded) {
                std::cout << "Done..." << std::endl;
                return true;
            }
            std::cerr << "Failed." << std::endl;
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
        auto path = fs::current_path();
        auto parentDir = path.parent_path();
        fs::current_path(parentDir);
        std::cout << "Relocated to parent dir..." << std::endl;
        return true;
    } catch(std::exception const& ex){
        std::cerr << "Failed to relocate working directory!" << std::endl;
        return false;
    }
}

bool find_cmake_project(){
    if (!fs::exists("CMakeLists.txt")){
        //std::cout << "Execution directory does not contain CMakeLists.txt!" << std::endl;
        //std::cout << "Looking in parent directory..." << std::endl;
        if (relocate()){
            if (fs::exists("CMakeLists.txt")){
                //std::cout << "Found CMakeLists!" << std::endl;
                return true;
            }else{
                std::cout << "Didn't find CMakeLists.txt!" << std::endl;
            }
        }
        return false;
    }
    //std::cout << "Found CMakeLists!" << std::endl;
    return true;
}

int main(int argc, char** argv) {
    std::string option = argc > 1 ? std::string(argv[1]):"";
    if (option == "help"){
        help();
        return EXIT_SUCCESS;
    }

    // check if cmake file found
    if (!find_cmake_project() || !check()){
        return EXIT_FAILURE;
    }

    if (run_option(option)){
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}

// UTILITY FUNCTIONS IMPLEMENTATION
static bool subfiles_exist(std::string path) {
    int count = 0;
    for (const auto& file : fs::directory_iterator(path)){
        if (!file.is_directory()){
            count++;
        }
    }
    return count > 0;
}

static std::string concat_sep(char sep, std::initializer_list<std::string> list) {
    auto vec = std::vector<std::string>{ list };
    return concat_sep(sep, vec);
}

static std::string concat_sep(char sep, std::vector<std::string>& vec){
    std::string result = "";
    for (const auto elem : vec) {
        result += elem;
        result += sep;
    }
    result.erase(result.length() - 1, 1);
    return result;
}

static bool make_dirs(std::string path){
    try {
        fs::create_directories(path);
        return true;
    } catch(fs::filesystem_error const& ex){
        std::cerr << "Failed to create directories!" << std::endl;
        std::cerr << ex.what() << std::endl;
        return false;
    }
}

static bool run_command(std::initializer_list<std::string> list){
    return run_command(std::vector<std::string>{list});
}

static bool run_command(std::vector<std::string> vec){
    if (!system(NULL)){
        std::cerr << "No command processor found!" << std::endl;
        return false;
    }

    std::string cmd = concat_sep(' ',vec);
    std::cout << ">> " << cmd << std::endl;

    int code = system(cmd.c_str());
    return code == 0;
}

static bool delete_recursive(std::string path){
    try {
        fs::remove_all(path);
        return true;
    } catch (fs::filesystem_error const& ex){
        std::cerr << "Failed to remove directories!" << std::endl;
        std::cerr << "TODO: You'll need to manually remove the folder for now." << std::endl;
        std::cerr << ex.what() << std::endl;
        return false;
    }
}

static void print_table(Table table, int padding){
    // calculate longest width first
    int width = 0;
    for (const auto& cmd : table){
        int len = cmd.first.length();
        if (len > width){
            width = len;
        }
    }
    width += padding;

    // print the table
    std::cout << std::endl;
    for (const auto& cmd : table){

        // pad string
        std::stringstream displayName;
        displayName << cmd.first;

        int spaces = width - cmd.first.length();
        for (int i = 0; i < spaces; i++){
            displayName << " ";
        }

        std::cout << displayName.str() << cmd.second << std::endl;
    }
}

static bool has_program(std::string program){
    const char* pathVar = std::getenv("PATH");
    if (pathVar == NULL){
        std::cerr << "Could not determine PATH variable" << std::endl;
        return false;
    }

#ifdef LINUX
    const char SEPERATOR = ':';
#elif defined(WINDOWS)
    const char SEPERATOR = ';';
#endif

    std::string path;
    std::istringstream stream(pathVar);
    while (std::getline(stream, path, SEPERATOR)) {
        auto folder = fs::path(path);
        if (fs::exists(folder)) {
            for (const auto& file : fs::directory_iterator(folder)) {
                std::string stem = file.path().stem().string();
                if (stem == program) {
                    return true;
                }
            }
        }
    }
    return false;
}

#ifdef LINUX
static bool make_archive(std::string output, std::vector<std::string>& files){

    // check file extension
    output = fs::path(output).replace_extension(".tar.xz").string();

    std::vector<std::string> cmd = {
        "tar",
        "cfJ",
        output
    };

    for (const auto& file: files){
        cmd.push_back(file);
    }

    return run_command(cmd);
}
#endif

#ifdef WINDOWS
static bool make_archive(std::string output, std::vector<std::string>& files){

    // check file extension
    output = fs::path(output).replace_extension(".zip").string();

    std::string fileArray = concat_sep(',',files);

    std::vector<std::string> ps = {
        "Compress-Archive",
        "-Path",
        fileArray
    };
    ps.push_back("-DestinationPath");
    ps.push_back(output);

    std::string psCommand = "\"" + concat_sep(' ', ps) + "\"";

    std::vector<std::string> cmd = {
        "powershell",
        "-c",
        psCommand
    };
    return run_command(cmd);
}
#endif

static std::string get_date_string(){
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    auto str = std::stringstream();
    str << std::put_time(&tm, "%d-%m-%Y-%H-%M");
    return str.str();
}
