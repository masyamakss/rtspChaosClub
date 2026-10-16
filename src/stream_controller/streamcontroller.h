#pragma once

#include "statistics_collector.h"

#include <gst/gst.h>
#include <iostream>

class StreamController
{
public:
    StreamController(InfoBus* infobus);

private:
    InfoBus* m_infobus = nullptr;

    RtspServer* rtspServer = nullptr;

    void createCommandFromWebServerHandler(const CreateSourceCommand&);
    void deleteCommandFromWebServerHandler(const DeleteSourceCommand&);
    void startCommandFromWebServerHandler(const StartSourceCommand&);
    void stopCommandFromWebServerHandler(const StopSourceCommand& stopCommand);

    std::uint64_t idCounter = 0;
    std::unordered_map<uint64_t, StreamData> m_observedStream;
    std::mutex m_streamsMutex;

    StatisticsCollector* m_StatisticsCollector = nullptr;
};
