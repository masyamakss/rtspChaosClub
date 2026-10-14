#include "statistics_collector.h"

StatisticsCollector::StatisticsCollector(const std::unordered_map<uint64_t, StreamData>& streams, std::mutex& mutex)    
    : m_streams(streams),
      m_streamsMutex(mutex)
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
    while (m_running)
    {
        m_cv.wait_for(lock, std::chrono::seconds(1), [this] { return !m_running.load(); });

        if (!m_running)
            break;

        lock.unlock();

        {
            mapLock.lock();
            
            for (const auto& it : m_streams)
            {
                std::cout
                << "Stream " << it.second.streamId
                << " generated: " << it.second.streamStatistic->generatedFrames.load()
                << " pushed: " << it.second.streamStatistic->pushedFrames.load()
                << " bytes: " << it.second.streamStatistic->pushedBytes.load()
                << '\n';
            }

            mapLock.unlock();
            
        }

        lock.lock();
    }
}