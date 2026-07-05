cmake_minimum_required(VERSION 3.10.0)
project(rtspChaosClub VERSION 0.1.0 LANGUAGES C CXX)

find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)
find_package(jsoncpp REQUIRED)

pkg_check_modules(GSTREAMER REQUIRED IMPORTED_TARGET gstreamer-1.0)

add_subdirectory(third_party/webview-0.12.0
                EXCLUDE_FROM_ALL)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/third_party/cmrc")

include(CMakeRC)

cmrc_add_resource_library(
    web_resources

    NAMESPACE web_resources

    resources/web/error.html
    resources/web/index.html
    resources/web/app.js
    resources/web/styles.css
)

add_executable(rtspChaosClub
                WIN32
                src/main.cpp
                src/webserver.h
                src/webserver.cpp
                src/application.h
                src/application.cpp
                src/webviewwindow.h
                src/webviewwindow.cpp
                src/embeddedresources.h
                src/embeddedresources.cpp
                src/streamcontroller.h
                src/streamcontroller.cpp
                src/sourcecommands.h
                src/infobus.h)

target_link_libraries(rtspChaosClub PRIVATE
    PkgConfig::GSTREAMER
    Threads::Threads
    webview::core
    web_resources
    jsoncpp_lib
)

target_include_directories(rtspChaosClub PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/cpp_httplib
    ${CMAKE_CURRENT_SOURCE_DIR}/src)