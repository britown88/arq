#pragma once

#include <unordered_map>
#include <exception>

template <typename T>
struct EnumPairHash
{
    int operator()(const std::pair<T, T> &pair) const
    {
      return static_cast<int>(pair.first)
           * static_cast<int>(T::COUNT)
           + static_cast<int>(pair.second);
    }
};

struct ArqException : public std::exception
{
   std::string s;
   ArqException(std::string ss) : s(ss) {}
   ~ArqException() throw () {} // Updated
   const char* what() const throw() { return s.c_str(); }
};
