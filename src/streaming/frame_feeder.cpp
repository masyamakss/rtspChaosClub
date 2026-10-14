#include "frame_feeder.h"

FrameFeeder::FrameFeeder(SyntheticVideoGenerator* generator,GstAppSrc* appsrc, std::weak_ptr<StreamStatistics> streamStatistics)
    : m_generator(generator),
      m_appsrc(appsrc),
      m_streamStatistics(streamStatistics)
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

    auto statistics = m_streamStatistics.lock();
    
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

        if (result == GST_FLOW_OK)
        {
            if (statistics)
            {
                statistics->pushedFrames.fetch_add(1);
                statistics->pushedBytes.fetch_add(frame.size());

                statistics->lastPushedAtUs.store(
                    std::chrono::duration_cast<std::chrono::microseconds>(
                        std::chrono::steady_clock::now().time_since_epoch()
                    ).count()
                );
            }
        }
        else
        {
            std::cerr << "Failed to push frame: " << result << '\n';
        }
    }
}