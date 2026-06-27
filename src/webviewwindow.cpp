#include "webviewwindow.h"

WebViewWindow::WebViewWindow() : m_webView(false, nullptr) 
{
    m_webView.set_title("RTSP Chaos Club");

    m_webView.set_size(1200, 800, WEBVIEW_HINT_NONE);
}

void WebViewWindow::openUrl(const std::string& url)
{
    m_webView.navigate(url);
}

void WebViewWindow::openErrorPage()
{
    const auto errorHtml = EmbeddedResources::loadText("resources/web/error.html");
    m_webView.set_html(errorHtml);
}

void WebViewWindow::run()
{
    m_webView.run();
}