#pragma once

#include "infobus.h"
#include "sourcecommands.h"

#include <gst/gst.h>
#include <iostream>

enum class StreamState
{
    Created,
    Running,
    Stopped,
    Error
};

struct streamData
{
    StreamState state;
    std::string mountPoint;
    CreateSourceCommand config;
};


class StreamController
{
public:
    StreamController(InfoBus* infobus);

private:
    InfoBus* m_infobus = nullptr;

    void startCommandFromWebServerHandler(const CreateSourceCommand&);

    std::unordered_map<std::uint64_t, streamData> m_observedStream;
};
