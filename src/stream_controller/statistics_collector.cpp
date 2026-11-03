#include "statistics_collector.h"

StatisticsCollector::StatisticsCollector(const std::unordered_map<uint64_t, StreamData>& streams, std::mutex& mutex, InfoBus* infobus)    
    : m_streams(streams),
      m_streamsMutex(mutex),
      m_infobus(infobus)
{}

StatisticsCollector::~StatisticsCollector()
{
    stop();
}

void StatisticsCollector::start()
{
    if (m_thread.joinable())
    {
        return;
    }
    {
        std::lock_guard<std::mutex> lock(m_waitMutex);
        m_running = true;
    }
    m_thread = std::thread(&StatisticsCollector::workerLoop, this);
}

void StatisticsCollector::stop()
{
    {
        std::lock_guard<std::mutex> lock(m_waitMutex);
        m_running = false;
    }
    m_cv.notify_one();
    if (m_thread.joinable())
    {
        m_thread.join();
    }
    
}

void StatisticsCollector::workerLoop()
{

    std::unique_lock<std::mutex> lock = std::unique_lock<std::mutex>(m_waitMutex);
    std::unique_lock<std::mutex> mapLock(m_streamsMutex, std::defer_lock);
    auto sampleTime = std::chrono::steady_clock::now();
    while (m_running)
    {
        m_cv.wait_for(lock, std::chrono::seconds(1), [this] { return !m_running.load(); });

        if (!m_running)
            break;

        lock.unlock();

        {
            m_copiedStreams.clear();

            mapLock.lock();
            
            sampleTime = std::chrono::steady_clock::now();

            for (const auto& it : m_streams)
            {
                m_copiedStreams.emplace_back(it.first, it.second.streamStatistic);
            }

            mapLock.unlock();
        }

        std::vector<StreamStatisticsSnapshot> snapShotVector;
        for (const auto& it : m_copiedStreams)
        {
            StreamStatisticsSnapshot snapShot{};
            snapShot.streamId = it.first;
            snapShot.lastGeneratedAtUs = it.second->lastGeneratedAtUs.load();
            snapShot.lastPushedAtUs = it.second->lastPushedAtUs.load();
            snapShot.pushedBytes = it.second->pushedBytes.load();
            snapShot.pushedFrames = it.second->pushedFrames.load();
            snapShot.generatedFrames = it.second->generatedFrames.load();

            if (!m_history.empty())
            {
                calculateMetrics(snapShot, sampleTime);
            }

            snapShotVector.emplace_back(snapShot);
        }

        TimedStatisticsSample timedStatisticsSample;
        timedStatisticsSample.streams = snapShotVector;
        timedStatisticsSample.timestamp = sampleTime;

        m_history.push_back(timedStatisticsSample);
        if (m_history.size() > 10)
        {
            m_history.pop_front();
        }

        StatisticsUpdatedEvent event{};
        event.streams.swap(snapShotVector);

        m_infobus->post(event);

        lock.lock();
    }
}

void StatisticsCollector::calculateMetrics(StreamStatisticsSnapshot& current, std::chrono::steady_clock::time_point currentTime)
{
    if (m_history.empty())
    {
        return;
    }

    for (const auto& stream : m_history.back().streams)
    {
        if (current.streamId == stream.streamId)
        {
            const double elapsedSeconds = std::chrono::duration<double>(currentTime - m_history.back().timestamp).count();
            if (elapsedSeconds <= 0.0 || current.generatedFrames < stream.generatedFrames)
            {
                break;
            }

            current.generatedFps = static_cast<double>(current.generatedFrames - stream.generatedFrames) / elapsedSeconds;
            current.pushedFps = static_cast<double>(current.pushedFrames - stream.pushedFrames) / elapsedSeconds;
            current.pushedMiBps = static_cast<double>(current.pushedBytes - stream.pushedBytes) / elapsedSeconds / (1024.0 * 1024.0);
            break;
        }
    }
}