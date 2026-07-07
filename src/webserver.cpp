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

        StartSourceCommand command;
        std::string errorOfFillingCommand = "";
        tryBuildStartSourceCommand(postRoot, command, errorOfFillingCommand);
        if (errorOfFillingCommand != "")
        {
            std::cerr << "JSON is broken, cant build command\n";
            response.status = 400;
            response.set_content("JSON in POST body is broken", "text/plain");
            return;
        }

        std::string contentToSet = "WebServer got command to start";

        response.set_content(contentToSet, "text/plain");

    });

}

void WebServer::tryBuildStartSourceCommand(Json::Value postRoot, StartSourceCommand& command, std::string& errorText)
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
