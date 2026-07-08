#ifndef WEBVIEWWINDOW_H
#define WEBVIEWWINDOW_H

#pragma once 

#include "webview/webview.h"
#include "embeddedresources.h"
#include "infobus.h"
#include <string>
#include <iostream>


class WebViewWindow
{
public:
    WebViewWindow(InfoBus* infoBus);

    void openUrl(const std::string& url);
    void openErrorPage();
    void run();

private:
    webview::webview m_webView;
    InfoBus* m_infoBus = nullptr;
};

#endif // WEBVIEWWINDOW_H
