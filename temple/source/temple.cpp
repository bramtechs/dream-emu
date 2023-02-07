#include <windows.h>
#include <stdio.h>
#include <malloc.h>
#include <xinput.h>
#include <windows.h>

#include "magma.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    InitWindow(640,480,"Temple game",hInstance,hPrevInstance,pCmdLine,nCmdShow);
    while (WindowIsOpen()) {
        std::cout << "hello world";
    }
    CloseWindow();
}
