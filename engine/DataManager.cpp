#include "DataManager.h"
#include "IOCContainer.h"
#include "JSON.h"
#include "AssetManager.h"

#include <unordered_map>

class DataManager::Impl
{
   std::unordered_map<std::string, std::function<void(Property&)>> m_loaders;

public:
   void registerDataTypeLoader(std::string type, std::function<void(Property&)> loader)
   {
      m_loaders.insert(std::make_pair(type, loader));
   }

   void loadData(std::string path)
   {
      auto am = IOC.resolve<AssetManager>();
      for(auto &file : am->getFiles(path))
      {
         auto prop = loadJSONToProperty(file);
         for(auto &obj : asArray(prop))
         {
            if(auto type = obj["type"].get<std::string>())
            {
               auto iter = m_loaders.find(*type);
               if(iter != m_loaders.end())
                  iter->second(obj);

            }
         }
      }
   }
};

DataManager::DataManager():pImpl(new Impl()){}
DataManager::~DataManager(){}

void DataManager::registerDataTypeLoader(std::string type, std::function<void(Property&)> loader){pImpl->registerDataTypeLoader(type, loader);}
void DataManager::loadData(std::string path){pImpl->loadData(path);}
