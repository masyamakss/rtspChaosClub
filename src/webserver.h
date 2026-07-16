#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "httplib.h"
#include "embeddedresources.h"
#include "sourcecommands.h"
#include "infobus.h"

#include <system_error>
#include "json/json.h"


class WebServer
{
public:
    explicit WebServer(InfoBus* infoBus, int preferredPort = 47774);
    ~WebServer();

    enum StartCode
    {
        StartUnknownError = 0,
        StartSuccess      = 1,
        StartBindFailed   = 2,
        StartThreadFailed = 3,
        StartListenFailed = 4,
        StartAlreadyRunning = 5,

        StartPortChanged  = 1 << 8
    };

    int start();
    void stop();

    /**
     * Returns the configured port before start().
     * Returns the actual listening port after a successful start().
     */
    int port() const {return m_port;}

private:
    int m_port;

    httplib::Server m_server;
    std::thread m_serverThread;

    void configureRoutes();

    void tryBuildCreateSourceCommand(Json::Value postRoot, CreateSourceCommand& command, std::string& errorText);

    void onCreatedCardHandler(const SourceCreatedEvent& event);
    void onFailedToCreateCardHandler(const SourceCreationFailedEvent& event);

    InfoBus* m_infoBus = nullptr;

    std::uint64_t m_nextRequestId = 1;

    std::deque<std::string> m_sseEvents;
    std::mutex m_sseMutex;
    std::condition_variable m_sseCv;
    void pushSseEvent(std::string message);
};

#endif // WEBSERVER_H
