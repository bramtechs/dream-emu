#ifdef WIN32
#include <windows.h>
#endif
#include <iostream>

void OpenSystemConsole(){
    #ifdef WIN32
    if (AllocConsole()) {
        freopen("CONIN$", "r", stdin);
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr); 
    }
    std::cout << "Allocated Windows console" << std::endl;
    #endif
    #ifndef WIN32
    std::cout << "Not using Windows, not showing console" << std::endl;
    #endif
}

void CloseSystemConsole(){
    #ifdef WIN32
    FreeConsole();
    #endif
}
