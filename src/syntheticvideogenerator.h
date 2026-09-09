#pragma once

#include <cstdint>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <cmath>
#include <cstring>
#include <chrono>
#include <iostream>
#include <fstream>

struct Point3D
{
    double x;
    double y;
    double z;
};

struct Point2D
{
    int x;
    int y;
};

class SyntheticVideoGenerator
{
public:
    SyntheticVideoGenerator(int width, int height, double cubeSpeed, double backgroundSpeed);

    ~SyntheticVideoGenerator();

    void start();
    void stop();
    bool takeLatestFrame(std::vector<std::uint8_t>& frame);

private:
    int m_width;
    int m_height;

    double m_cubeSpeed;
    double m_backgroundSpeed;

    double m_cubeAngle = 0.0;
    double m_backgroundOffset = 0.0;

    std::vector<std::uint8_t> m_frontFrame;
    std::vector<std::uint8_t> m_backFrame;
    std::size_t frameSize;

    std::mutex m_frameMutex;

    std::thread m_thread;
    std::atomic_bool m_running{false};

    void generateFrame(std::vector<std::uint8_t>& frame, double deltaTime);
    void setPixel(std::vector<uint8_t>& frame, int x, int y, uint8_t b, uint8_t g, uint8_t r);
    void initializeBackground(double initialOffset = 0);
    void drawBackground(std::vector<std::uint8_t>& frame, double deltaTime);
    void drawCube(std::vector<uint8_t> &frame, double deltaTime);
    void drawLine(std::vector<uint8_t> &frame, const Point2D &start, const Point2D &end);
    void saveFrameToPpm(const std::vector<uint8_t> &frame, const std::__cxx11::string &fileName);
    Point3D rotatePoint(const Point3D& point, double angle);
    Point2D project(const Point3D& point);
    Point3D m_cubePosition{0.0, 0.0, 5.0};
    Point3D m_cubeVertices[8] =
    {
        {-1.0, -1.0, -1.0},
        { 1.0, -1.0, -1.0},
        { 1.0,  1.0, -1.0},
        {-1.0,  1.0, -1.0},
        
        {-1.0, -1.0,  1.0},
        { 1.0, -1.0,  1.0},
        { 1.0,  1.0,  1.0},
        {-1.0,  1.0,  1.0}
    };
    const int squareSize = 100;
    double m_backgroundOffsetX = 0.0;
    double m_backgroundOffsetY = 0.0;
    std::vector<uint8_t> m_frameBackground;
    
    void workerLoop();

    bool m_hasFrame = false;
};