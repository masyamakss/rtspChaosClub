#pragma once

#include <atomic>
#include <thread>
#include <chrono>
#include <cstring>
#include <iostream>
#include <vector>

#include "syntheticvideogenerator.h"

#include <gst/app/gstappsrc.h>


class FrameFeeder
{
public:
    FrameFeeder(SyntheticVideoGenerator* generator, GstAppSrc* appsrc, std::weak_ptr<StreamStatistics> streamStatistics);

    ~FrameFeeder();

    void start();
    void stop();

private:
    void workerLoop();

private:
    SyntheticVideoGenerator* m_generator = nullptr;
    GstAppSrc* m_appsrc = nullptr;

    std::thread m_thread;
    std::atomic_bool m_running{false};

    std::weak_ptr<StreamStatistics> m_streamStatistics;
};