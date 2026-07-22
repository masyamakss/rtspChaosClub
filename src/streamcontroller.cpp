#include "streamcontroller.h"

StreamController::StreamController(InfoBus *infobus) : m_infobus(infobus)
{
    m_infobus->subscribe<CreateSourceCommand>([this](const CreateSourceCommand& startCommand){startCommandFromWebServerHandler(startCommand);});
    m_infobus->subscribe<DeleteSourceCommand>([this](const DeleteSourceCommand& deleteCommand){deleteCommandFromWebServerHandler(deleteCommand);});
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

void StreamController::deleteCommandFromWebServerHandler(const DeleteSourceCommand& deleteCommand)
{
    if (m_observedStream.count(deleteCommand.streamId) > 0)
    {
        m_observedStream.erase(deleteCommand.streamId);
    }

    DeletedSourceEvent deletedSource;
    deletedSource.streamId = deleteCommand.streamId;

    m_infobus->post(deletedSource);
}
