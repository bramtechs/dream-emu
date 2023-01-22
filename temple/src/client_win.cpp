#include "client.hpp"
#include "windows.h"

// some windows duct-taping
INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	PSTR lpCmdLine, INT nCmdShow)
{
	return main();
}
