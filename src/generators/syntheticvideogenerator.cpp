#include "syntheticvideogenerator.h"

SyntheticVideoGenerator::SyntheticVideoGenerator(int width, int height, double cubeSpeed, double backgroundSpeed)
    : m_width(width),
      m_height(height),
      m_cubeSpeed(cubeSpeed),
      m_backgroundSpeed(backgroundSpeed)
{
    m_frameSize = static_cast<std::size_t>(m_width) * static_cast<std::size_t>(m_height) * 3;

    m_frontFrame.resize(m_frameSize);
    m_backFrame.resize(m_frameSize);
    m_frameBackground.resize(m_frameSize);

    initializeBackground();
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
    auto previousTime = std::chrono::steady_clock::now();
    while (m_running)
    {
        const auto currentTime = std::chrono::steady_clock::now();
        const double deltaTime = std::chrono::duration<double>(currentTime - previousTime).count();
        previousTime = currentTime;

        generateFrame(m_backFrame, deltaTime);

        {
            std::lock_guard<std::mutex> lock(m_frameMutex);

            m_frontFrame.swap(m_backFrame);
            m_hasFrame = true;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }
}

void SyntheticVideoGenerator::generateFrame(std::vector<std::uint8_t>& frame, double deltaTime)
{
    drawBackground(frame, deltaTime);
    drawCube(frame, deltaTime);
}

bool SyntheticVideoGenerator::takeLatestFrame(std::vector<std::uint8_t>& frame)
{
    if (frame.size() != m_frameSize)
    {
        frame.resize(m_frameSize);
    }

    std::lock_guard<std::mutex> lock(m_frameMutex);

    if (!m_hasFrame)
    {
        return false;
    }

    frame.swap(m_frontFrame);

    m_hasFrame = false;

    return true;
}

void SyntheticVideoGenerator::setPixel(std::vector<uint8_t>& frame, int x, int y, uint8_t b, uint8_t g, uint8_t r)
{
    if (x < 0 || y < 0 || x >= m_width || y >= m_height)
    {
        return;
    }

    size_t indexOfPixel = (static_cast<size_t>(y) * m_width + static_cast<size_t>(x)) * 3;

    frame[indexOfPixel] = b;
    frame[indexOfPixel + 1] = g;
    frame[indexOfPixel + 2] = r;
}

Point2D SyntheticVideoGenerator::project(const Point3D& point)
{
    const double focalLength = 500.0;

    return {static_cast<int>(m_width / 2.0 + focalLength * point.x / point.z), static_cast<int>(m_height / 2.0 - focalLength * point.y / point.z)};
}

void SyntheticVideoGenerator::initializeBackground(double initialOffset)
{
    const int squareSize = 80;
    const int period = squareSize * 2;
    m_backgroundOffset += m_backgroundSpeed * initialOffset;

    const int offset = static_cast<int>(m_backgroundOffset) % period;

    for (int y = 0; y < m_height; ++y)
    {
        for (int x = 0; x < m_width; ++x)
        {
            int patternX = x % period;
            int patternY = (y - offset) % period;

            if (patternY < 0)
            {
                patternY += period;
            }

            const int squareX = patternX / squareSize;
            const int squareY = patternY / squareSize;

            const bool isWhite = (squareX + squareY) % 2 == 0;
            const std::uint8_t value = isWhite ? 255 : 0;

            setPixel(m_frameBackground, x, y, value, value, value);
        }
    }
}

Point3D SyntheticVideoGenerator::rotatePoint(const Point3D& point, double angle)
{
    Point3D rotated{};

    rotated.x = point.x * std::cos(angle) + point.z * std::sin(angle);

    rotated.y = point.y;

    rotated.z = -point.x * std::sin(angle) + point.z * std::cos(angle);

    return rotated;
}

void SyntheticVideoGenerator::drawBackground(std::vector<std::uint8_t>& frame, double deltaTime)
{
    m_backgroundOffset += m_backgroundSpeed * deltaTime;

    const int offsetX = static_cast<int>(m_backgroundOffset) % m_width;

    const int offsetY = static_cast<int>(m_backgroundOffset) % m_height;

    constexpr std::size_t pixelSize = 3;

    const std::size_t rowSize = static_cast<std::size_t>(m_width) * pixelSize;

    const std::size_t offsetBytes = static_cast<std::size_t>(offsetX) * pixelSize;

    const std::size_t firstPartSize = rowSize - offsetBytes;

    // Какая строка исходника должна попасть в y = 0
    int sourceY = offsetY == 0 ? 0 : m_height - offsetY;

    for (int y = 0; y < m_height; ++y)
    {
        const std::uint8_t* sourceRow = m_frameBackground.data() + static_cast<std::size_t>(sourceY) * rowSize;

        std::uint8_t* destinationRow = frame.data() + static_cast<std::size_t>(y) * rowSize;

        if (offsetX == 0)
        {
            std::memcpy(destinationRow, sourceRow, rowSize);
        }
        else
        {
            // ABC | DE
            //
            // сначала:
            // ___ABC

            std::memcpy(destinationRow + offsetBytes, sourceRow, firstPartSize);

            // потом:
            // DE_ABC

            std::memcpy(destinationRow, sourceRow + firstPartSize, offsetBytes);
        }

        ++sourceY;

        if (sourceY == m_height)
        {
            sourceY = 0;
        }
    }
}

void SyntheticVideoGenerator::drawLine(std::vector<std::uint8_t>& frame,
                                       const Point2D& start,
                                       const Point2D& end)
{
    int x = start.x;
    int y = start.y;

    const int dx = std::abs(end.x - start.x);
    const int dy = std::abs(end.y - start.y);

    const int stepX = start.x < end.x ? 1 : -1;
    const int stepY = start.y < end.y ? 1 : -1;

    int error = dx - dy;

    while (true)
    {
        setPixel(frame, x, y, 0, 0, 255);

        if (x == end.x && y == end.y)
            break;

        const int error2 = error * 2;

        if (error2 > -dy)
        {
            error -= dy;
            x += stepX;
        }

        if (error2 < dx)
        {
            error += dx;
            y += stepY;
        }
    }
}

void SyntheticVideoGenerator::drawCube(std::vector<std::uint8_t>& frame, double deltaTime)
{
    m_cubeAngle += m_cubeSpeed * deltaTime;

    Point2D projectedVertices[8];

    for (int i = 0; i < 8; ++i)
    {
        Point3D rotated = rotatePoint(m_cubeVertices[i], m_cubeAngle);

        rotated.x += m_cubePosition.x;
        rotated.y += m_cubePosition.y;
        rotated.z += m_cubePosition.z;

        projectedVertices[i] = project(rotated);
    }

    const int edges[12][2] =
    {
        {0, 1}, {1, 2}, {2, 3}, {3, 0},
        {4, 5}, {5, 6}, {6, 7}, {7, 4},
        {0, 4}, {1, 5}, {2, 6}, {3, 7}
    };

    for (int i = 0; i < 12; ++i)
    {
        drawLine(frame, projectedVertices[edges[i][0]], projectedVertices[edges[i][1]]);
    }
}