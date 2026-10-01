cmake_minimum_required(VERSION 3.10.0)

project(rtspChaosClub VERSION 0.1.0 LANGUAGES C CXX)

find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)
find_package(jsoncpp REQUIRED)

pkg_check_modules(GSTREAMER REQUIRED IMPORTED_TARGET
    gstreamer-1.0
    gstreamer-app-1.0
    gstreamer-rtsp-server-1.0
)

add_subdirectory(
    third_party/webview-0.12.0
    EXCLUDE_FROM_ALL
)

list(APPEND CMAKE_MODULE_PATH
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/cmrc"
)

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

    # Root
    src/root/main.cpp
    src/root/application.h
    src/root/application.cpp

    # Web
    src/web/webserver.h
    src/web/webserver.cpp
    src/web/webviewwindow.h
    src/web/webviewwindow.cpp
    src/web/embeddedresources.h
    src/web/embeddedresources.cpp

    # Stream controller
    src/stream_controller/streamcontroller.h
    src/stream_controller/streamcontroller.cpp

    # InfoBus
    src/infobus/infobus.h
    src/infobus/sourcecommands.h

    # Streaming
    src/streaming/rtspserver.h
    src/streaming/rtspserver.cpp
    src/streaming/frame_feeder.h
    src/streaming/frame_feeder.cpp

    # Generators
    src/generators/syntheticvideogenerator.h
    src/generators/syntheticvideogenerator.cpp
)

target_compile_features(rtspChaosClub PRIVATE
    cxx_std_17
)

target_link_libraries(rtspChaosClub PRIVATE
    PkgConfig::GSTREAMER
    Threads::Threads
    webview::core
    web_resources
    jsoncpp_lib
)

target_include_directories(rtspChaosClub PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/cpp_httplib

    ${CMAKE_CURRENT_SOURCE_DIR}/src

    ${CMAKE_CURRENT_SOURCE_DIR}/src/root
    ${CMAKE_CURRENT_SOURCE_DIR}/src/web
    ${CMAKE_CURRENT_SOURCE_DIR}/src/stream_controller
    ${CMAKE_CURRENT_SOURCE_DIR}/src/infobus
    ${CMAKE_CURRENT_SOURCE_DIR}/src/streaming
    ${CMAKE_CURRENT_SOURCE_DIR}/src/generators
)