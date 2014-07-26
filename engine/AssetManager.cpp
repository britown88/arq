#include "AssetManager.h"

#include <fstream>
#include <string>
#include <sstream>

#include "GnUtilities.h"
#include "IOCContainer.h"
#include "boost\algorithm\string.hpp"
#include "boost\filesystem.hpp"

std::string readFullFile(const std::string &path)
{
  std::ifstream in(path, std::ios::in | std::ios::binary);

  if(in)
  {
    std::string contents;
    in.seekg(0, std::ios::end);
    contents.resize(in.tellg());
    in.seekg(0, std::ios::beg);
    in.read(&contents[0], contents.size());
    in.close();
    return(contents);
  }

  throw(ArqException("Unable to open file."));
}

AssetManager::AssetManager()
{
   auto st = IOC.resolve<StringTable>();

   std::ifstream assetFile("assets/assetindex");

   if(!assetFile.is_open())
      throw(ArqException("Unable to open asset index file."));

   m_rootNode = AssetNode(st->get("assets"));

   std::string line;
   std::vector<std::string> parts;
   while(std::getline(assetFile, line))
   {
      auto *currentNode = &m_rootNode;
      boost::split(parts, line, boost::is_any_of(" "));

      int partCount = parts.size();

      for(int i = 0; i < partCount; ++i)
      {
         auto &part = parts[i];
         auto nodeName = st->get(part);

         std::stringstream strim;
         strim << *currentNode->path << "/" << part;
         auto fullPath = st->get(strim.str());

         if(i == partCount - 1)
         {
            //last part, check for a file
            if(!boost::filesystem::is_directory(
               boost::filesystem::current_path() / *fullPath))
            {
               currentNode->files.insert(*fullPath);
            }
         }
         else
         {
            auto iter = currentNode->children.find(nodeName);

            if(iter == currentNode->children.end())
            {  
               AssetNode nextNode(fullPath);
               auto item = currentNode->children.insert(std::make_pair(nodeName, nextNode));
               currentNode = &item.first->second;
            }
            else
            {
               currentNode = &iter->second;
            }
         }
      }
   }

   assetFile.close();   
}

std::set<std::string> &AssetManager::getFiles(std::string path)
{
   auto st = IOC.resolve<StringTable>();
   return getFiles(st->get(path));
}

std::set<std::string> &AssetManager::getFiles(InternString path)
{
   auto st = IOC.resolve<StringTable>();

   std::vector<std::string> parts;
   boost::split(parts, *path, boost::is_any_of("/"));

   AssetNode *currentNode = &m_rootNode;

   if(parts.empty() || st->get(parts[0]) != currentNode->path)
      throw ArqException("Invalid Path");

   for(int i = 1; i < parts.size(); ++i)
   {
      auto iter = currentNode->children.find(st->get(parts[i]));
      if(iter != currentNode->children.end())
         currentNode = &iter->second;
      else
         throw ArqException("Invalid Path");
   }

   return currentNode->files;
}