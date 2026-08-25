#include "syntheticvideogenerator.h"

SyntheticVideoGenerator::SyntheticVideoGenerator(int width, int height, double cubeSpeed, double backgroundSpeed)
    : m_width(width),
      m_height(height),
      m_cubeSpeed(cubeSpeed),
      m_backgroundSpeed(backgroundSpeed)
{
    const std::size_t frameSize = static_cast<std::size_t>(m_width) * static_cast<std::size_t>(m_height) * 3;

    m_frontFrame.resize(frameSize);
    m_backFrame.resize(frameSize);
}

void SyntheticVideoGenerator::start()
{
    if (m_running)
    {
        return;
    }

    m_running = true;

    m_thread = std::thread(&SyntheticVideoGenerator::workerLoop, this);
}

void SyntheticVideoGenerator::stop()
{
    if (!m_running)
    {
        return;
    }

    if (m_thread.joinable())
    {
        m_thread.join();
    }
}

SyntheticVideoGenerator::~SyntheticVideoGenerator()
{
    stop();
}

void SyntheticVideoGenerator::workerLoop()
{
    while (m_running)
    {
        generateFrame(m_backFrame);

        {
            std::lock_guard<std::mutex> lock(m_frameMutex);

            m_frontFrame.swap(m_backFrame);
            m_hasFrame = true;
        }

        std::this_thread::sleep_for(
            std::chrono::milliseconds(33)
        );
    }
}

void SyntheticVideoGenerator::generateFrame(
    std::vector<std::uint8_t>& frame)
{
    std::fill(
        frame.begin(),
        frame.end(),
        50
    );
}