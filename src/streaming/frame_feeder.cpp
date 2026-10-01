#include "frame_feeder.h"

FrameFeeder::FrameFeeder(
    SyntheticVideoGenerator* generator,
    GstAppSrc* appsrc)
    : m_generator(generator),
      m_appsrc(appsrc)
{}

void FrameFeeder::start()
{
    if (m_running)
    {
        return;
    }

    m_running = true;

    m_thread = std::thread([this]()
    {
        workerLoop();
    });
}

void FrameFeeder::stop()
{
    m_running = false;

    if (m_thread.joinable())
    {
        m_thread.join();
    }
}

FrameFeeder::~FrameFeeder()
{
    stop();
}

void FrameFeeder::workerLoop()
{
    std::vector<uint8_t> frame;
    
    while (m_running)
    {
        if (!m_generator->takeLatestFrame(frame))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        GstBuffer* buffer = gst_buffer_new_allocate(nullptr, frame.size(), nullptr);

        if (buffer == nullptr)
        {
            continue;
        }

        GstMapInfo mapInfo{};

        if (!gst_buffer_map(buffer, &mapInfo, GST_MAP_WRITE))
        {
            gst_buffer_unref(buffer);
            continue;
        }

        std::memcpy(mapInfo.data, frame.data(), frame.size());

        gst_buffer_unmap(buffer, &mapInfo);

        const GstFlowReturn result = gst_app_src_push_buffer(m_appsrc, buffer);

        if (result != GST_FLOW_OK)
        {
            std::cerr << "Failed to push frame: " << result << '\n';
        }
    }
}