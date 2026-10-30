#pragma once

#include <atomic>
#include <cstdint>

struct StreamStatistics
{
    // Количество кадров
    std::atomic<std::uint64_t> generatedFrames{0};
    std::atomic<std::uint64_t> pushedFrames{0};

    // Переданные данные
    std::atomic<std::uint64_t> pushedBytes{0};

    // Индексы кадров
    std::atomic<std::uint64_t> lastGeneratedFrameId{0};
    std::atomic<std::uint64_t> lastPushedFrameId{0};

    // Время последних операций (микросекунды)
    std::atomic<std::int64_t> lastGeneratedAtUs{0};
    std::atomic<std::int64_t> lastPushedAtUs{0};
};

struct StreamStatisticsSnapshot
{
    uint64_t streamId;
    uint64_t generatedFrames;
    uint64_t pushedFrames;
    uint64_t pushedBytes;
    int64_t lastGeneratedAtUs;
    int64_t lastPushedAtUs;
};