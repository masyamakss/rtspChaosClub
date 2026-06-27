#include "application.h"

Application::Application() : m_server(new WebServer())
{}

Application::~Application()
{
    m_server->stop();
    delete m_server;
}

int Application::startLaboratory()
{
    GError* gstError = nullptr;

    if (!gst_init_check(nullptr, nullptr, &gstError))
    {
        std::string error = "GStreamer ran into a problem during start";

        if (gstError != nullptr)
        {
            error = gstError->message;
            g_error_free(gstError);
            gstError = nullptr;
        }

        std::cerr << error << '\n';

        window.openErrorPage();
        window.run();
        return 1;
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

    const std::string url =
    "http://127.0.0.1:" +
    std::to_string(m_server->port()) +
    "/";

    std::cout << "Web server: " << url << '\n';

    window.openUrl(url);
    window.run();

    return 0;
}
