#pragma once

#include <cstdint>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>

class SyntheticVideoGenerator
{
public:
    SyntheticVideoGenerator(int width, int height, double cubeSpeed, double backgroundSpeed);

    ~SyntheticVideoGenerator();

    void start();
    void stop();

private:
    int m_width;
    int m_height;

    double m_cubeSpeed;
    double m_backgroundSpeed;

    double m_cubeAngle = 0.0;
    double m_backgroundOffset = 0.0;

    std::vector<std::uint8_t> m_frontFrame;
    std::vector<std::uint8_t> m_backFrame;

    std::mutex m_frameMutex;

    std::thread m_thread;
    std::atomic_bool m_running{false};

    void generateFrame(std::vector<std::uint8_t>& frame);

    void workerLoop();

    bool m_hasFrame = false;
};