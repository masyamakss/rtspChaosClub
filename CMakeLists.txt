cmake_minimum_required(VERSION 3.10.0)
project(rtspChaosClub VERSION 0.1.0 LANGUAGES C CXX)

find_package(PkgConfig REQUIRED)

pkg_check_modules(GSTREAMER REQUIRED IMPORTED_TARGET gstreamer-1.0)

add_executable(rtspChaosClub main.cpp)

target_link_libraries(rtspChaosClub PRIVATE
    PkgConfig::GSTREAMER
)

target_include_directories(rtspChaosClub PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/third_party/cpp_httplib)