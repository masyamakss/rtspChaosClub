#pragma once

#include <functional>
#include <typeindex>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <unordered_map>

class InfoBus
{
public:
      InfoBus();
      ~InfoBus();

      InfoBus(const InfoBus&) = delete;
      InfoBus& operator=(const InfoBus&) = delete;
      
      template <typename Event>
      void subscribe(std::function<void(const Event&)> handler);
      
      template <typename Event>
      void post(const Event& event);

private:
      void workerLoop();

      std::unordered_map<std::type_index, std::vector<std::function<void(const void*)>>> m_handlers;

      std::queue<std::function<void()>> m_tasks;

      std::mutex m_mutex;
      std::condition_variable m_cv;
      std::thread m_workerThread;

      bool m_stopping = false;
};

inline InfoBus::InfoBus()
{
      m_workerThread = std::thread(&InfoBus::workerLoop, this);
}

inline InfoBus::~InfoBus()
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stopping = true;
    }
    m_cv.notify_one();

    if (m_workerThread.joinable())
    {
        m_workerThread.join();
    }
}

inline void InfoBus::workerLoop()
{
      while (true)
      {
            std::unique_lock<std::mutex> lock(m_mutex);
            
            m_cv.wait(lock, [this]()
            {
                  return m_stopping || !m_tasks.empty();
            });

            if (m_stopping && m_tasks.empty())
            {
                  return;
            }

            std::function<void()> currentTask = std::move(m_tasks.front());
            m_tasks.pop();
            lock.unlock();
            currentTask();
      }
}

template<typename Event>
void InfoBus::post(const Event &event)
{
      std::type_index typeIndex(typeid(Event));
      
      std::vector<std::function<void(const void*)>> handlersVector;

      {
            std::lock_guard<std::mutex> lock(m_mutex);

            auto it = m_handlers.find(typeIndex);
            if (it == m_handlers.end())
            {
                  return;
            }

            handlersVector = it->second;
      }

      std::function<void()> funcForQueue([eventCopy = event, handlersVector]()
      {
            for (const auto& func : handlersVector)
            {
                  func(&eventCopy);
            }
      });
      {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_tasks.push(funcForQueue);
      }

      m_cv.notify_one();
}

template<typename Event>
void InfoBus::subscribe(std::function<void (const Event &)> handler)
{
        std::type_index typeIndex(typeid (Event));

        std::function<void(const void*)> handlerWrapper([handler](const void* rawEvent)
        {
            const Event* castedEvent = static_cast<const Event*>(rawEvent);
            handler(*castedEvent);
        });

        std::lock_guard<std::mutex> lock(m_mutex);
        m_handlers[typeIndex].push_back(handlerWrapper);
}
