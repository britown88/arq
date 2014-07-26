#pragma once

#include <unordered_map>
#include <set>

#include "StringTable.h"
#include "boost\optional.hpp"

std::string readFullFile(const std::string &path);

struct AssetNode
{
   InternString path;
   std::unordered_map<InternString, AssetNode> children;
   std::set<std::string> files;

   AssetNode(){}
   AssetNode(InternString path):path(path){}
   
   boost::optional<AssetNode &> get(InternString dir)
   {
      auto iter = children.find(dir);
      if(iter != children.end())
         return iter->second;
      else
         return boost::none;
   }
};

class AssetManager
{
   AssetNode m_rootNode;
public:
   AssetManager();

   std::set<std::string> &getFiles(InternString path);
   std::set<std::string> &getFiles(std::string path);
};