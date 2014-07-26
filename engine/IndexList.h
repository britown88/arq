#pragma once

#include <vector>
#include <memory>

class IndexList
{
public:
   IndexList(){}
   IndexList(const IndexList &rhs):indices(rhs.indices){}
   ~IndexList(){}
   std::vector<char> indices;
   static const int indexSize = 1;
};

class IBO
{
public:
   virtual const int getHandle() const=0;
   virtual const int getCount() const=0;
};

typedef std::shared_ptr<IBO> IBOPtr;

std::shared_ptr<IBO> createIBO(IndexList &iList);