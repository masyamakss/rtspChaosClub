#include <iostream>
#include "application.h"

#ifdef _WIN32

int WINAPI WinMain(
    HINSTANCE,
    HINSTANCE,
    LPSTR,
    int)
{
    Application app;

    return app.startLaboratory();
}

#else

int main(int argc, char** argv){

    std::cout << "Hello, from rtspChaosClub!\n";

    Application app;

    return app.startLaboratory();
}

#endif
