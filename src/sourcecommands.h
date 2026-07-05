#pragma once

#include <string>

struct StartSourceCommand
{
    std::string mode;
    std::string resolution;

    int cubeSpeed = 0;
    int backgroundSpeed = 0;

    std::string fileName;
};