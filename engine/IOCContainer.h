#pragma once

#include <unordered_map>
#include <typeinfo>
#include <string>
#include <memory>

#include <boost/optional.hpp>

#include "TypeInfo.h"


class IDeleter
{
public:
   virtual ~IDeleter(){}
   virtual void destroy(void *ptr)=0;
};

template<typename T>
class PolymorphicDeleter : public IDeleter
{
public:
   static PolymorphicDeleter &instance()
   {
      static PolymorphicDeleter t;
      return t;
   }

   void destroy(void *ptr)
   {
      delete (T*)ptr;
   }
};

struct IOCPtr
{
   IDeleter *deleter;
   void *ptr;

   IOCPtr(){}
   IOCPtr(IOCPtr && other)
      :deleter(other.deleter), ptr(other.ptr)
   {
      other.deleter = nullptr;
   }

   ~IOCPtr()
   {
      if(deleter)
         deleter->destroy(ptr);
   }
};

class IOCContainer
{
   std::unordered_map<size_t, IOCPtr> m_typeInstanceMap;

public:
   template<typename T>
   boost::optional<T&> resolve()
   {
      auto iter = m_typeInstanceMap.find(TypeInfo<T>::instance().ID);

      if(iter == m_typeInstanceMap.end())
         return boost::none;
      else
         return *(T*)iter->second.ptr;
   }

   void clear()
   {
      m_typeInstanceMap.clear();
   }

   template<typename T>
   void remove()
   {
      m_typeInstanceMap.erase(TypeInfo<T>::instance().ID);
   }

   template<typename T>
   void add(std::unique_ptr<T> obj)
   {
      IOCPtr ptr;
      ptr.ptr = obj.release();
      ptr.deleter = &PolymorphicDeleter<T>::instance();

      m_typeInstanceMap.insert(
         std::make_pair(TypeInfo<T>::instance().ID, std::move(ptr))
         );
   }
};


extern IOCContainer IOC;
