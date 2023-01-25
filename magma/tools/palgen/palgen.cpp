#include <iostream>
#include <fstream>
#include <string>

#include "raylib.h"

std::pair<bool, std::string> check_paths(const char* input, const char* output) {
	if (!FileExists(input)) {
		std::string msg = TextFormat("Input file %s doesn't exist.", input);
		return std::make_pair(false, msg);
	}
	std::string ext = GetFileExtension(output);
	if (ext != ".pal") {
		return std::make_pair(false, "Output file needs have extension .pal");
	}
	const char* base = GetPrevDirectoryPath(output);
	if (!DirectoryExists(base)) {
		std::string msg = TextFormat("Output path %s doesn't exist.", base);
		return std::make_pair(false, msg);
	}
	return std::make_pair(true, "");
}

const char* get_pal_name(const char* output) {
	std::string path = GetFileNameWithoutExt(output);
	std::string prefix = "pal_";
	if (path.at(3) == '_' && path.length() > 4) {
		path = path.substr(4);
	}
	return TextToUpper(path.c_str());
}

std::pair<bool, std::string> extract_from_path(const char* input, const char* output) {

	Image img = LoadImage(input);
	if (img.width == 0) {
		return std::make_pair(false, "Image couldn't be loaded!");
	}

	int colorCount = -1;
	Color* colors = LoadImagePalette(img, 256, &colorCount);
	if (colorCount == -1) {
		return std::make_pair(false, "Could load image colors!");
	}

	// write colors to file
	std::ofstream out;
	out.open(output);

	// write header
	out << "JASC-PAL" << std::endl;
	out << "0100" << std::endl;
	out << colorCount << std::endl;

	// write each color
	for (int i = 0; i < colorCount; i++) {
		Color col = colors[i];
		out << std::to_string(col.r) << " "
			<< std::to_string(col.g) << " "
			<< std::to_string(col.b)
			<< std::endl;
	}

	out.close();

	std::string msg = TextFormat("Extracted %d colors from %s to %s", colorCount, input, output);
	return std::make_pair(true, msg);
}

int execute_with_args(int argc, char* argv[]) {
	if (argc == 0) {
		std::cout << "No arguments given!" << std::endl;
		std::cout << "Expected input and output path" << std::endl;
		return -1;
	}

	if (argc >= 2) {
		const char* input = argv[0];
		const char* output = argv[1];
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

	char* test_argv[] = {
		"X:\\temple\\raw_assets\\spr_block_0.png",
		"X:\\temple\\raw_assets\\pal_warm1.pal",
	};
	int test_argc = 2;

	// return execute_with_args(argc, argv);
	return execute_with_args(test_argc, test_argv);
}
