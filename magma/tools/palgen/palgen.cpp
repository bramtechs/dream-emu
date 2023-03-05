#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <format>
#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Basic usage (see HDR discussion below for HDR usage):
//    int x,y,n;
//    unsigned char *data = stbi_load(filename, &x, &y, &n, 0);
//    // ... process data if not NULL ...
//    // ... x = width, y = height, n = # 8-bit components per pixel ...
//    // ... replace '0' with '1'..'4' to force that many components per pixel
//    // ... but 'n' will always be the number that it would have been if you said 0
//    stbi_image_free(data);
//
// Standard parameters:
//    int *x                 -- outputs image width in pixels
//    int *y                 -- outputs image height in pixels
//    int *channels_in_file  -- outputs # of image components in image file
//    int desired_channels   -- if non-zero, # of image components requested in result

namespace fs = std::filesystem;

inline const char* CSTR(const fs::path& path){
    return path.string().c_str();
}

struct Color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};

struct Image {
    int width;
    int height;
    int comps;
    unsigned char* data;

    Image(const fs::path& fileName) {
        data = stbi_load(CSTR(fileName),&width, &height,&comps,4);
    }

    bool loaded(){
        return data;
    }

    std::vector<Color> extract_palette() {
        std::vector<Color> colors;
        for (int i = 0; i < width*height; i++){
            Color col = {
                data[i*comps+0], // r
                data[i*comps+1], // g
                data[i*comps+2], // b
                data[i*comps+3], // a
            };
            colors.emplace_back(col);
        }
        return colors;
    }

    ~Image(){
        if (data) stbi_image_free(data);
    }
};

std::pair<bool, std::string> check_paths(const fs::path& input, const fs::path& output) {
    if (!fs::exists(input)){
        std::string msg = std::format("Input file {} doesn't exist.", input);
        return std::make_pair(false, msg);
    }
    if (output.extension() != ".pal") {
        return std::make_pair(false, "Output file needs have extension .pal");
    }
    return std::make_pair(true, "");
}

std::string get_pal_name(const fs::path& path) {
    std::string fileName = path.stem().string();

    // strip prefix if it has one
    std::string prefix = "pal_";
    if (prefix == fileName.substr(0, prefix.length())) {
        fileName = fileName.substr(prefix.length());
    }

    // to uppercase
    std::transform(fileName.begin(), fileName.end(),fileName.begin(), ::toupper);
    return fileName;
}

std::pair<bool, std::string> extract_from_path(const fs::path& input, const fs::path& output) {

    Image img = Image(input);
    if (img.loaded()) {
        return std::make_pair(false, "Image couldn't be loaded!");
    }

    std::vector<Color> colors = img.extract_palette();

    // write colors to file
    std::ofstream out;
    out.open(output);

    // write header
    out << "JASC-PAL" << std::endl;
    out << "0100" << std::endl;
    out << colors.size() << std::endl;

    // write each color
    for (const Color& col : colors){
        out << std::to_string(col.r) << " "
            << std::to_string(col.g) << " "
            << std::to_string(col.b)
            << std::endl;
    }
    out.close();

    std::string msg = std::format("Extracted {} colors from {} to {}", colorCount, input, output);
    return std::make_pair(true, msg);
}

int execute_with_args(int argc, char* argv[]) {
    if (argc == 0) {
        std::cout << "No arguments given!" << std::endl;
        std::cout << "Expected input and output path" << std::endl;
        return -1;
    }

    if (argc >= 2) {
        const char* input = argv[argc - 2];
        const char* output = argv[argc - 1];

        auto check = check_paths(input, output);
        if (check.first) {
            auto result = extract_from_path(input, output);
            std::cout << result.second << std::endl;
            return result.first ? 0 : -1;
        }
        std::cout << check.second << std::endl;
        return -1;
    }
    std::cout << "Unexpected amount of arguments" << std::endl;
    return -1;
}

int main(int argc, char* argv[]) {

    //char* test_argv[] = {
    //    "X:\\temple\\raw_assets\\spr_block_0.png",
    //    "X:\\temple\\raw_assets\\pal_warm1.pal",
    //};
    //int test_argc = 2;

    return execute_with_args(argc, argv);
    // return execute_with_args(test_argc, test_argv);
}
