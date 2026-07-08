#include "streamcontroller.h"

StreamController::StreamController(InfoBus *infobus) : m_infobus(infobus)
{
    m_infobus->subscribe<StartSourceCommand>([this](const StartSourceCommand& startCommand){startCommandFromWebServerHandler(startCommand);});
}

void StreamController::startCommandFromWebServerHandler(const StartSourceCommand& startCommand)
{
    std::cerr << "КОМАНДА ОТ ВЕБ СЕРВЕРА ОБРАБОТАНА, mode = "
              << startCommand.mode << '\n';
}
