#pragma once

#include "thread"
#include "unordered_map"

#include "gst/rtsp-server/rtsp-server.h"

struct RtspSourceData
{
    GstRTSPMediaFactory* factory = nullptr;
    bool mounted = false;
};


class RtspServer
{
public:
    RtspServer();
    ~RtspServer();

    bool addSource(const std::string& mountPoint);
    bool startSource(const std::string& mountPoint);

private:
    GstRTSPServer* m_server = nullptr;
    GstRTSPMountPoints* m_mountPoints = nullptr;

    GMainContext* m_context = nullptr;
    GMainLoop* m_loop = nullptr;

    std::thread m_thread;

    guint m_attachId = 0;

    std::unordered_map<std::string, RtspSourceData> m_sources;
};