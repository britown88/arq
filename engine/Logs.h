#pragma once

#include <memory>
#include <vector>
#include <iostream>
#include <sstream>
#include <string>
#include <deque>

enum class LogLevel : unsigned int
{
   Debug=0,
   Info,
   Warning,
   Error
};

struct LogObj
{
   LogLevel level;
   std::string msg;
};

class LogStreamObj
{
   std::stringstream m_stream;
   LogLevel m_level;

   LogStreamObj(const LogStreamObj&rhs);

   bool m_needsPost;

public:
   LogStreamObj(LogLevel level, std::string reporter);
   LogStreamObj(LogStreamObj && rhs)
      :m_level(rhs.m_level), m_stream(std::move(rhs.m_stream)), m_needsPost(true){rhs.m_needsPost = false;}

   ~LogStreamObj();

   template <typename T>
   LogStreamObj& operator <<(T const& data)
   {
      m_stream << data;
      return *this;
   }

};

class LogManager
{
   class Impl;
   std::unique_ptr<Impl> pImpl;
public:
   LogManager();
   ~LogManager();

   void pushLog(LogObj log);
   std::deque<LogObj> getLogs();

};

namespace Logs
{
   LogStreamObj d(std::string reporter);
   LogStreamObj i(std::string reporter);
   LogStreamObj w(std::string reporter);
   LogStreamObj e(std::string reporter);
};