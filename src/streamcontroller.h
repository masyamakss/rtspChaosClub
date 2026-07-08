#pragma once

#include "infobus.h"
#include "sourcecommands.h"

#include <iostream>

class StreamController
{
public:
    StreamController(InfoBus* infobus);

private:
    InfoBus* m_infobus = nullptr;

    void startCommandFromWebServerHandler(const StartSourceCommand&);
};
