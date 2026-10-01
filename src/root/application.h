#ifndef APPLICATION_H
#define APPLICATION_H

#include "webserver.h"
#include "webviewwindow.h"
#include "embeddedresources.h"
#include "streamcontroller.h"
#include "gst/gst.h"


class Application
{
public:
    Application();
    ~Application();
    Application& operator=(const Application&) = delete;
    Application(const Application&) = delete;

    int startLaboratory();

private:

    InfoBus m_infoBus;
    
    WebServer* m_server = nullptr;

    WebViewWindow window;

   

    StreamController streamController;

};

#endif // APPLICATION_H
