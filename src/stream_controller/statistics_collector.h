#pragma once

#include <atomic>
#include <thread>
#include <mutex>
#include <unordered_map>

#include "infobus.h"
#include "rtspserver.h"
#include "stream_statistics.h"

enum class StreamState
{
    Created,
    Running,
    Stopped,
    Error
};

struct StreamData
{
    int streamId;
    StreamState state;
    std::string mountPoint;
    CreateSourceCommand config;
    std::shared_ptr<StreamStatistics> streamStatistic;
};


class StatisticsCollector
{
public:
    StatisticsCollector(const std::unordered_map<uint64_t, StreamData>& streams, std::mutex& streamMutex);
    ~StatisticsCollector();

    void start();
    void stop();

private:
    void workerLoop();

    std::thread m_thread;
    std::atomic_bool m_running{false};

    std::mutex& m_streamsMutex;
    const std::unordered_map<uint64_t, StreamData>& m_streams;

    std::thread m_thread;
    std::atomic_bool m_running{false};
    std::mutex m_waitMutex;
    std::condition_variable m_cv;
};