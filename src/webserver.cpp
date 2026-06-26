#include "webserver.h"

WebServer::WebServer(int preferredPort)
{
    m_port = preferredPort;

    configureRoutes();
}

WebServer::~WebServer()
{
    stop();
}

int WebServer::start()
{
    int result = StartSuccess;

    if (m_serverThread.joinable())
    {
        return result = StartAlreadyRunning;
    }

    bool isBound = false;
    int tmpPort = m_port;

    for (;tmpPort < m_port + 10; tmpPort++)
    {
        isBound = m_server.bind_to_port("127.0.0.1", tmpPort);
        if (isBound)
        {
            if (tmpPort != m_port)
            {
                m_port = tmpPort;
                result |= StartPortChanged;
            }
            break;
        }
        else
        {
            m_server.stop();
        }
    }

    if (!isBound)
    {
        result = StartBindFailed;
        return result;
    }

    try
    {
        m_serverThread = std::thread([this]()
        {
            m_server.listen_after_bind();
        });
    }
    catch(const std::system_error& error)
    {
        std::cerr << error.what() << std::endl;
        return result = StartThreadFailed;
    }

    if (!m_server.is_running())
    {
        if (m_serverThread.joinable())
        {
            m_serverThread.join();
        }

        return result = StartListenFailed;
    }

    return result;
}

void WebServer::stop()
{
    m_server.stop();

    if (m_serverThread.joinable())
    {
        m_serverThread.join();
    }

}

void WebServer::configureRoutes()
{
    m_server.Get("/", [](const httplib::Request& request, httplib::Response& response)
    {
        response.set_content("RTSP Chaos Club is running\n","text/plain");
    });

}
