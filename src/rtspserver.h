#pragma once

#include <iostream>
#include "thread"
#include "unordered_map"

#include "sourcecommands.h"

#include "gst/rtsp-server/rtsp-server.h"
#include <gst/app/gstappsrc.h>

struct RtspSourceData
{
    GstRTSPMediaFactory* factory = nullptr;
    GstRTSPMedia* media = nullptr;
    GstAppSrc* appsrcData = nullptr;
    
    bool mounted = false;
    std::string mountPoint;
    CreateSourceCommand configInfo;
};


class RtspServer
{
public:
    RtspServer();
    ~RtspServer();

    bool addSource(const std::string& mountPoint, const CreateSourceCommand& configInfo);
    bool startSource(const std::string& mountPoint);

private:
    GstRTSPServer* m_server = nullptr;
    GstRTSPMountPoints* m_mountPoints = nullptr;

    GMainContext* m_context = nullptr;
    GMainLoop* m_loop = nullptr;

    std::thread m_thread;

    guint m_attachId = 0;

    std::unordered_map<std::string, RtspSourceData> m_sources;

    static void onMediaConfigure(GstRTSPMediaFactory* factory, GstRTSPMedia* media, gpointer userData);
    static void onMediaUnprepared(GstRTSPMedia* media, gpointer userData);
};