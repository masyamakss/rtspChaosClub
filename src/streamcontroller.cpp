#include "streamcontroller.h"

StreamController::StreamController(InfoBus *infobus) : m_infobus(infobus)
{
    m_infobus->subscribe<CreateSourceCommand>([this](const CreateSourceCommand& startCommand){startCommandFromWebServerHandler(startCommand);});
}

void StreamController::startCommandFromWebServerHandler(const CreateSourceCommand& startCommand)
{
    std::cerr << "КОМАНДА ОТ ВЕБ СЕРВЕРА ОБРАБОТАНА, mode = " << startCommand.mode << '\n';
    
    idCounter += 1;

    StreamData data{};
    data.config = startCommand;
    data.mountPoint = "/stream-" + std::to_string(idCounter);
    data.state = StreamState::Created;
    data.streamId = idCounter;

    m_observedStream[idCounter] = data;

    SourceCreatedEvent createdCard{};
    createdCard.mountPoint = data.mountPoint;
    createdCard.requestId = startCommand.requestId;
    createdCard.streamId = idCounter;

    m_infobus->post(createdCard);
    return;
}
