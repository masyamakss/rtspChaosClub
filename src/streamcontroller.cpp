#include "streamcontroller.h"

StreamController::StreamController(InfoBus *infobus) : m_infobus(infobus)
{
    m_infobus->subscribe<CreateSourceCommand>([this](const CreateSourceCommand& createCommand){createCommandFromWebServerHandler(createCommand);});
    m_infobus->subscribe<DeleteSourceCommand>([this](const DeleteSourceCommand& deleteCommand){deleteCommandFromWebServerHandler(deleteCommand);});
    m_infobus->subscribe<StartSourceCommand>([this](const StartSourceCommand& startCommand){startCommandFromWebServerHandler(startCommand);});

    rtspServer = new RtspServer();

}

void StreamController::createCommandFromWebServerHandler(const CreateSourceCommand& createCommand)
{
    std::cerr << "КОМАНДА ОТ ВЕБ СЕРВЕРА ОБРАБОТАНА, mode = " << createCommand.mode << '\n';
    
    idCounter += 1;

    StreamData data{};
    data.config = createCommand;
    data.mountPoint = "/stream-" + std::to_string(idCounter);
    data.state = StreamState::Created;
    data.streamId = idCounter;

    m_observedStream[idCounter] = data;

    SourceCreatedEvent createdCard{};
    createdCard.mountPoint = data.mountPoint;
    createdCard.requestId = createCommand.requestId;
    createdCard.streamId = idCounter;

    rtspServer->addSource(data.mountPoint, data.config);

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

void StreamController::startCommandFromWebServerHandler(const StartSourceCommand& startCommand)
{
    auto it = m_observedStream.find(startCommand.streamId);
    if (it == m_observedStream.end())
    {
        //Вернуть ошибку
        return;
    }
    if (!rtspServer->startSource(it->second.mountPoint))
    {
        //Вернуть ошибку
        return;
    }

    it->second.state = StreamState::Running;

    StartSourceEvent startedEvent{};
    startedEvent.streamId = startCommand.streamId;
    m_infobus->post(startedEvent);
}