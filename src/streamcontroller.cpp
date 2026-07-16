#include "streamcontroller.h"

StreamController::StreamController(InfoBus *infobus) : m_infobus(infobus)
{
    m_infobus->subscribe<CreateSourceCommand>([this](const CreateSourceCommand& startCommand){startCommandFromWebServerHandler(startCommand);});
}

void StreamController::startCommandFromWebServerHandler(const CreateSourceCommand& startCommand)
{
    std::cerr << "КОМАНДА ОТ ВЕБ СЕРВЕРА ОБРАБОТАНА, mode = "
              << startCommand.mode << '\n';

    if (m_observedStream.count(startCommand.requestId) > 0)
    {
        // СДЕЛАТЬ ПОСТ С ОШИБКОЙ СОЗДАНИЯ КАРТОЧКИ
    }
}
