# rtspChaosClub

A C++17/Linux project for experimenting with reliability and failure handling in real-time media streaming.

The goal is to build a small lab where RTSP/RTP streaming behavior can be observed, tested and eventually stressed under different failure scenarios.

## Why

Real-time media systems often behave well under ideal conditions but become much more interesting when connections are interrupted, packets are delayed or components restart.

I started this project to explore those failure modes and build tooling around stream diagnostics and reliability.

## Current focus

- RTSP/RTP media transport
- stream state and diagnostics
- failure/reconnect behavior
- internal asynchronous event handling
- lightweight local control and monitoring UI

## Tech

- C++17
- Linux
- CMake
- FFmpeg / GStreamer
- HTTP / networking
- multithreading

## Status

Work in progress.

The project is being developed incrementally as a practical environment for learning and experimenting with media transport reliability.

## Screenshots

Coming soon.
