#pragma once 

#include <functional>
#include <typeindex>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
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