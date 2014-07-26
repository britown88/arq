#pragma once

#include <typeinfo>
#include <string>
#include <memory>

template <typename T>
struct TypeInfo
{
   size_t ID;

   TypeInfo()
   {
      ID = std::hash<std::string>()(typeid(T).name());
   }

   static TypeInfo &instance()
   {
      static TypeInfo t;
      return t;
   }
};