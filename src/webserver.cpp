#include "webserver.h"

WebServer::WebServer(InfoBus* infoBus, int preferredPort)
    : m_infoBus(infoBus),
      m_port(preferredPort)
{
    configureRoutes();
    infoBus->subscribe<SourceCreatedEvent>([this](SourceCreatedEvent createdCard){onCreatedCardHandler(createdCard);});
    infoBus->subscribe<SourceCreationFailedEvent>([this](SourceCreationFailedEvent failedCard){onFailedToCreateCardHandler(failedCard);});
    infoBus->subscribe<DeletedSourceEvent>([this](DeletedSourceEvent deletedCard){onDeletedCardHandler(deletedCard);});
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
        m_server.stop();
        return result = StartThreadFailed;
    }

    m_server.wait_until_ready();

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

        std::cerr << "GET / received\n";
        const auto mainMenuHtml = EmbeddedResources::loadText("resources/web/index.html");

        std::cout
            << "index.html size: "
            << mainMenuHtml.size()
            << '\n';

        response.set_content(mainMenuHtml,"text/html; charset=UTF-8");
    });

    m_server.Get("/styles.css", [](const httplib::Request& request, httplib::Response& response)
    {   

        std::cerr << "GET /styles.css received\n";
        const auto stylesCss = EmbeddedResources::loadText("resources/web/styles.css");

        std::cout << "style.css size: " << stylesCss.size() << '\n';

        response.set_content(stylesCss,"text/css; charset=UTF-8");
    });

    m_server.Get("/app.js", [](const httplib::Request& request, httplib::Response& response)
    {   

        std::cerr << "GET /app.js received\n";
        const auto appJs = EmbeddedResources::loadText("resources/web/app.js");

        std::cout << "app.js size: " << appJs.size() << '\n';

        response.set_content(appJs,"application/javascript; charset=UTF-8");
    });

    m_server.Post("/api/test", [this](const httplib::Request& request, httplib::Response& response)
    {   
        auto jsonBody = request.body;
        std::cerr << "POST body: " << request.body << '\n';

        Json::Value postRoot;
        Json::Reader postReader;
        if (!postReader.parse(jsonBody, postRoot))
        {
            std::cerr << "JSON in POST body is broken \n";
            response.status = 400;
            response.set_content("JSON in POST body is broken", "text/plain");
            return;
        } 

        if (!postRoot.isMember("mode"))
        {
            std::cerr << "JSON in POST body is broken \n";
            response.status = 400;
            response.set_content("JSON in POST body is broken - no mode", "text/plain");
            return;
        }

        if (postRoot["mode"].asString() != "fileMode" && postRoot["mode"].asString() != "synthMode")
        {
            std::cerr << "JSON in POST body is broken - unknown mode \n";
            response.status = 400;
            response.set_content("JSON in POST body is broken - unknown mode", "text/plain");
            return;
        }

        CreateSourceCommand command;
        std::string errorOfFillingCommand = "";
        tryBuildCreateSourceCommand(postRoot, command, errorOfFillingCommand);
        if (errorOfFillingCommand != "")
        {
            std::cerr << "JSON is broken, cant build command\n";
            response.status = 400;
            response.set_content("JSON in POST body is broken", "text/plain");
            return;
        }

        const std::uint64_t requestId = m_nextRequestId++;

        command.requestId = requestId;


        Json::Value responseJson;
        responseJson["accepted"] = true;
        responseJson["requestId"] = Json::UInt64(requestId);

        Json::StreamWriterBuilder writer;
        response.set_content(Json::writeString(writer, responseJson), "application/json");

        m_infoBus->post(command);

    });

    m_server.Get("/api/events",[this](const httplib::Request&,httplib::Response& response)
    {
        response.set_header("Cache-Control", "no-cache");
        response.set_header("Connection", "keep-alive");

        response.set_chunked_content_provider("text/event-stream",[this](std::size_t,httplib::DataSink& sink)
        {
            std::string message;

            {
                std::unique_lock<std::mutex> lock(m_sseMutex);

                const bool eventReceived = m_sseCv.wait_for(lock, std::chrono::seconds(5),
                    [this]()
                    {
                        return !m_sseEvents.empty();
                    });
                
                if (eventReceived)
                {
                    message = std::move(m_sseEvents.front());
                    m_sseEvents.pop_front();
                    std::cerr << "CV woke up succesfully\n";
                    std::cerr << "bytes = " << message.size() << "\nMESSAGE:\n" << message << '\n';
                }
                else
                {
                    message = ": keep-alive\n\n";
                }
            }

            const bool writeResult = sink.write(message.data(), message.size());

            
            return writeResult;
        });
    });

    m_server.Post("/api/source/delete",[this](const httplib::Request& request, httplib::Response& response)
    {
        auto jsonBody = request.body;
        std::cerr << "POST body: " << request.body << '\n';

        Json::Value postRoot;
        Json::Reader postReader;
        if (!postReader.parse(jsonBody, postRoot))
        {
            std::cerr << "JSON in POST body is broken \n";
            response.status = 400;
            response.set_content("JSON in POST body is broken", "text/plain");
            return;
        } 

        if (!postRoot["streamId"].isUInt64())
        {
            response.status = 400;
            response.set_content("Invalid streamId", "text/plain");
            return;
        }

        const std::uint64_t streamId = postRoot["streamId"].asUInt64();

        response.status = 200;
        response.set_content("Valid streamId", "text/plain");

        DeleteSourceCommand deleteCard;
        deleteCard.streamId = streamId;
        m_infoBus->post(deleteCard);

        std::cerr << "streamId: " << streamId << '\n';
    });
}

void WebServer::tryBuildCreateSourceCommand(Json::Value postRoot, CreateSourceCommand& command, std::string& errorText)
{
    errorText.clear();

    if (!postRoot.isMember("mode") || !postRoot["mode"].isString())
    {
        errorText = "mode in postRoot is missing or invalid";
        return;
    }

    const std::string mode = postRoot["mode"].asString();

    if (mode == "fileMode")
    {
        if (!postRoot.isMember("fileName") || !postRoot["fileName"].isString())
        {
            errorText = "fileName in postRoot is missing or invalid";
            return;
        }

        command.mode = "fileMode";
        command.fileName = postRoot["fileName"].asString();

        return;
    }

    if (mode == "synthMode")
    {
        if (!postRoot.isMember("resolution") || !postRoot["resolution"].isString())
        {
            errorText = "resolution in postRoot is missing or invalid";
            return;
        }

        if (!postRoot.isMember("cubeSpeed") || !postRoot["cubeSpeed"].isInt())
        {
            errorText = "cubeSpeed in postRoot is missing or invalid";
            return;
        }

        if (!postRoot.isMember("backgroundSpeed") || !postRoot["backgroundSpeed"].isInt())
        {
            errorText = "backgroundSpeed in postRoot is missing or invalid";
            return;
        }

        command.mode = "synthMode";
        command.resolution = postRoot["resolution"].asString();
        command.cubeSpeed = postRoot["cubeSpeed"].asInt();
        command.backgroundSpeed = postRoot["backgroundSpeed"].asInt();

        return;
    }

    errorText = "unknown mode in postRoot";
}

void WebServer::pushSseEvent(std::string message)
{
    {
        std::lock_guard<std::mutex> lock(m_sseMutex);

        m_sseEvents.push_back(std::move(message));
    }

    m_sseCv.notify_one();
}

void WebServer::onCreatedCardHandler(const SourceCreatedEvent& event)
{

    std::cerr << "InfoBus sent info of created source\n";
    Json::Value json;

    json["requestId"] = Json::UInt64(event.requestId);

    json["streamId"] = Json::UInt64(event.streamId);

    json["mountPoint"] = event.mountPoint;

    Json::StreamWriterBuilder writer;
    writer["indentation"] = "";

    const std::string jsonText = Json::writeString(writer, json);

    std::string message;

    message += "event: source-created\n";
    message += "data: ";
    message += jsonText;
    message += "\n\n";

    pushSseEvent(std::move(message));
}

void WebServer::onDeletedCardHandler(const DeletedSourceEvent& event)
{

    std::cerr << "InfoBus sent info of deleted source\n";
    Json::Value json;

    json["streamId"] = Json::UInt64(event.streamId);

    Json::StreamWriterBuilder writer;
    writer["indentation"] = "";

    const std::string jsonText = Json::writeString(writer, json);

    std::string message;

    message += "event: source-deleted\n";
    message += "data: ";
    message += jsonText;
    message += "\n\n";

    pushSseEvent(std::move(message));
}

void WebServer::onFailedToCreateCardHandler(const SourceCreationFailedEvent& event)
{
    Json::Value json;

    json["requestId"] = Json::UInt64(event.requestId);

    json["reason"] = event.reason;

    Json::StreamWriterBuilder writer;
    writer["indentation"] = "";

    const std::string jsonText = Json::writeString(writer, json);

    std::string message;

    message += "event: source-creation-failed\n";
    message += "data: ";
    message += jsonText;
    message += "\n\n";

    pushSseEvent(std::move(message));
}