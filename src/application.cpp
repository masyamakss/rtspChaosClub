#include "application.h"

Application::Application(int argc, char **argv)
{
    GstArgc = argc;
    GstArgv = argv;
    m_server = new WebServer();
}

Application::~Application()
{
    m_server->stop();
    delete m_server;
}

int Application::startLaboratory()
{

    GError* gstError = nullptr;

    if (!gst_init_check(&GstArgc, &GstArgv, &gstError))
    {
        std::string error = "GStreamer ran into a problem during start";

        if (gstError != nullptr)
        {
            error = gstError->message;
            g_error_free(gstError);
            error = nullptr;
        }

        // ВЫВОД ВЕБ ОКНА С ОШИБКОЙ
    }

    const int serverStartResult = m_server->start();

    const int status = serverStartResult & 0xFF;

    if (status != WebServer::StartSuccess)
    {
        std::cerr << "Failed to start web server. Code: "
                  << status << '\n';

        return status;
    }

    if ((serverStartResult & WebServer::StartPortChanged) != 0)
    {
        std::cout << "Preferred port was unavailable.\n";
    }

    std::cout << "Web server: http://127.0.0.1:" << m_server->port() << "/\n";

    return 0;
}
