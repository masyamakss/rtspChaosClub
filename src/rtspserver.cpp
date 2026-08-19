#include "rtspserver.h"

RtspServer::RtspServer()
{
    m_context = g_main_context_new();

    m_loop = g_main_loop_new(m_context, false);

    m_server = gst_rtsp_server_new();

    gst_rtsp_server_set_service(m_server, "34345");

    m_mountPoints = gst_rtsp_server_get_mount_points(m_server);

    m_attachId = gst_rtsp_server_attach(m_server, m_context);

    if (m_attachId == 0)
    {
        return;
    }

    m_thread = std::thread([this]()
    {
        g_main_loop_run(m_loop);
    });
}

RtspServer::~RtspServer()
{
    if (m_loop != nullptr)
    {
        g_main_loop_quit(m_loop);
    }

    if (m_thread.joinable())
    {
        m_thread.join();
    }

        if (m_attachId != 0 && m_context != nullptr)
    {
        GSource* source =
            g_main_context_find_source_by_id(
                m_context,
                m_attachId
            );

        if (source != nullptr)
        {
            g_source_destroy(source);
        }

        m_attachId = 0;
    }
    
    if (m_mountPoints != nullptr)
    {
        for (auto& [mountPoint, sourceData] : m_sources)
        {
            if (sourceData.factory != nullptr)
            {
                g_object_unref(sourceData.factory);
                sourceData.factory = nullptr;
            }
        }

        m_sources.clear();

        g_object_unref(m_mountPoints);
        m_mountPoints = nullptr;
    }

    if (m_server != nullptr)
    {
        g_object_unref(m_server);
        m_server = nullptr;
    }

    if (m_loop != nullptr)
    {
        g_main_loop_unref(m_loop);
        m_loop = nullptr;
    }

    if (m_context != nullptr)
    {
        g_main_context_unref(m_context);
        m_context = nullptr;
    }
}

bool RtspServer::addSource(const std::string& mountPoint)
{
    if (m_sources.find(mountPoint) != m_sources.end())
    {
        return false;
    }

    RtspSourceData sourceData{};
    sourceData.factory = gst_rtsp_media_factory_new();

    gst_rtsp_media_factory_set_shared(sourceData.factory, true);


    gst_rtsp_media_factory_set_launch(
        sourceData.factory,
        "( appsrc name=source "
        "! videoconvert "
        "! x264enc tune=zerolatency "
        "! rtph264pay name=pay0 pt=96 )"
    );

    m_sources.emplace(mountPoint, sourceData);
    
    return true;
}

bool RtspServer::startSource(const std::string& mountPoint)
{
    auto it = m_sources.find(mountPoint);

    if (it == m_sources.end())
    {
        return false;
    }

    RtspSourceData& sourceData = it->second;

    if (sourceData.factory == nullptr || sourceData.mounted)
    {
        return false;
    }

    g_object_ref(sourceData.factory);

    gst_rtsp_mount_points_add_factory(m_mountPoints, mountPoint.c_str(), sourceData.factory);

    sourceData.mounted = true;
    return true;
}