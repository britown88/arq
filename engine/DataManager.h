#pragma once

#include <memory>
#include <string>
#include <functional>

#include "PropertyMap.h"

class DataManager
{
   class Impl;
   std::unique_ptr<Impl> pImpl;
public:
   DataManager();
   ~DataManager();

   void registerDataTypeLoader(std::string type, std::function<void(Property&)> loader);
   void loadData(std::string path);
};