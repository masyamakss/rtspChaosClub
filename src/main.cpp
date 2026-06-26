#include <iostream>
#include "application.h"

int main(int argc, char** argv){

    std::cout << "Hello, from rtspChaosClub!\n";

    Application app(argc, argv);

    return app.startLaboratory();
}
