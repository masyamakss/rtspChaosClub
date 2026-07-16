#pragma once

#include <string>

struct CreateSourceCommand
{
    std::uint64_t requestId;

    std::string mode;
    std::string resolution;

    int cubeSpeed = 0;
    int backgroundSpeed = 0;

    std::string fileName;
};

struct SourceCreatedEvent
{
    std::uint64_t requestId;
    std::uint64_t streamId;
    std::string mountPoint;
};

struct SourceCreationFailedEvent
{
    std::uint64_t requestId;
    std::string reason;
};