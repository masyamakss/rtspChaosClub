#include "webviewwindow.h"

WebViewWindow::WebViewWindow(InfoBus* infoBus) : m_webView(true, nullptr)
{
    m_infoBus = infoBus;
    m_webView.set_title("RTSP Chaos Club");

    m_webView.set_size(1200, 800, WEBVIEW_HINT_NONE);
}

void WebViewWindow::openUrl(const std::string& url)
{
    std::cerr << "WebView navigating to: " << url << '\n';
    
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
