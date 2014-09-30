#include "Logs.h"
#include "IOCContainer.h"

#include <mutex>
#include <deque>

class LogManager::Impl
{
   std::deque<LogObj> m_logs;
   std::mutex m;
public:
   Impl(){}
   ~Impl(){}

   void pushLog(LogObj log)
   {
      m.lock();
      m_logs.push_back(log);
      if(m_logs.size() >= 50)
         m_logs.pop_front();

      m.unlock();
   }
   std::deque<LogObj> getLogs()
   {
      m.lock();
      auto ret = std::deque<LogObj>(m_logs);

      m.unlock();

      return ret;
   }
};

LogManager::LogManager():pImpl(new Impl()){}
LogManager::~LogManager(){}

void LogManager::pushLog(LogObj log){pImpl->pushLog(log);}
std::deque<LogObj> LogManager::getLogs(){return pImpl->getLogs();}


LogStreamObj::LogStreamObj(LogLevel level, std::string reporter)
   :m_level(level), m_needsPost(true)
{
   m_stream << reporter << ": ";
}
LogStreamObj::~LogStreamObj()
{
   if(!m_needsPost)
      return;

   if(auto lm = IOC.resolve<LogManager>())
   {
      LogObj obj;
      obj.level = m_level;
      obj.msg = m_stream.str();

      lm->pushLog(obj);
   }
}

LogStreamObj Logs::d(std::string reporter){return LogStreamObj(LogLevel::Debug, reporter);}
LogStreamObj Logs::i(std::string reporter){return LogStreamObj(LogLevel::Info, reporter);}
LogStreamObj Logs::w(std::string reporter){return LogStreamObj(LogLevel::Warning, reporter);}
LogStreamObj Logs::e(std::string reporter){return LogStreamObj(LogLevel::Error, reporter);}