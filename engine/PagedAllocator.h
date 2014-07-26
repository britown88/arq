#pragma once

#include <intrin.h>

static const size_t ObjectsPerPage = 2048;

class SpinLock
{
   volatile long locked; 
public:
   SpinLock():locked(0){}

   bool tryLock();
   void lock();
   void unlock();
   

};

template<typename T>
struct Page
{
   Page *next;

   char data[sizeof(T)*ObjectsPerPage];
};

struct Link
{
   Link *next;
};

template<typename T>
class PagedAllocator
{
   Page<T> *head;
   Link *freeList;

   SpinLock m;

   void createPage()
   {
      auto p = new Page<T>();
      p->next = head;
      head = p;

      T *iter = (T*)p->data;
      for(size_t i = 0; i < ObjectsPerPage; ++i)
      {
         auto liter = ((Link*)iter);
         liter->next = freeList;
         freeList = liter;
         ++iter;
      }
   }

public:
   static PagedAllocator &instance()
   {
      static PagedAllocator i;
      return i;
   }

   PagedAllocator():head(nullptr), freeList(nullptr){}

   T *alloc()
   {
      m.lock();
      if(!freeList)
         createPage();

      auto out = freeList;
      freeList = freeList->next;

      m.unlock();

      return (T*)out;
   }

   void free(T *t)
   {
      m.lock();
      auto lt = (Link*)t;
      lt->next = freeList;
      freeList = lt;
      m.unlock();
   }
};