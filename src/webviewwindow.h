#ifndef WEBVIEWWINDOW_H
#define WEBVIEWWINDOW_H

#pragma once 

#include <string>
#include "webview/webview.h"
#include "embeddedresources.h"


class WebViewWindow
{
public:
    WebViewWindow();

    void openUrl(const std::string& url);
    void openErrorPage();
    void run();

private:
    webview::webview m_webView;
};

#endif // WEBVIEWWINDOW_H
