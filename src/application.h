#ifndef APPLICATION_H
#define APPLICATION_H

#include "webserver.h"
#include "gst/gst.h"


class Application
{
public:
    Application(int argc, char **argv);
    ~Application();
    Application& operator=(const Application&) = delete;
    Application(const Application&) = delete;

    int startLaboratory();

private:

    WebServer* m_server = nullptr;

    int GstArgc;
    char **GstArgv;

};

#endif // APPLICATION_H
