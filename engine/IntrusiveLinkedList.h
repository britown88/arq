#pragma once

#include <iterator>
#include <stddef.h>

namespace std
{
   template<typename T>
   T begin(const std::pair<T, T> &p){return p.first;}

   template<typename T>
   T end(const std::pair<T, T> &p){return p.second;}
}

template<typename T>
std::pair<typename T::reverse_iterator, typename T::reverse_iterator> reversed_view(T &ref)
{
   return std::make_pair(ref.rbegin(), ref.rend());
}


template<class Parent, class Member>
inline ptrdiff_t offset_from_pointer_to_member(const Member Parent::* ptr_to_member)
{
   //msvc compliant compilers use their the first 32 bits as offset (even in 64 bit mode)
   union caster_union
   {
      const Member Parent::* ptr_to_member;
      int offset;
   } caster;
   caster.ptr_to_member = ptr_to_member;
   return ptrdiff_t (caster.offset);
}

template<class Parent, class Member>
inline Parent *parentFromMember(Member *member, const Member Parent::* ptr_to_member)
{
   return static_cast<Parent*>
      (
        static_cast<void*>
        (
          static_cast<char*>(static_cast<void*>(member)) - offset_from_pointer_to_member(ptr_to_member)
        )
      );
}

template<class Parent, class Member>
inline const Parent *parentFromMember(const Member *member, const Member Parent::* ptr_to_member)
{
   return static_cast<const Parent*>
      (
        static_cast<const void*>
        (
          static_cast<const char*>(static_cast<const void*>(member)) - offset_from_pointer_to_member(ptr_to_member)
        )
      );
}

struct IntrusiveListHook
{
   IntrusiveListHook()
      : next(nullptr), prev(nullptr)
   {

   }
   ~IntrusiveListHook()
   {
      unlink();
   }
   IntrusiveListHook(IntrusiveListHook&& rhs)
   {
      next = rhs.next;
      prev = rhs.prev;
      if (prev) prev->next = this;
      if (next) next->prev = this;
      rhs.next = rhs.prev = nullptr;
   }
   IntrusiveListHook& operator=(IntrusiveListHook&& rhs)
   {
      unlink();
      next = rhs.next;
      prev = rhs.prev;
      if (prev) prev->next = this;
      if (next) next->prev = this;
      rhs.next = rhs.prev = nullptr;

      return *this;
   }
   void unlink()
   {
      if (next) next->prev = prev;
      if (prev) prev->next = next;
      next = prev = nullptr;
   }
   bool inList() const
   {
      return next != nullptr;
   }
   IntrusiveListHook* next, *prev;

private:
//   IntrusiveListHook(IntrusiveListHook const &);
//   IntrusiveListHook& operator=(IntrusiveListHook const &);

};


template <typename T, IntrusiveListHook T::*hookMember>
class IntrusiveListIterator
{
   IntrusiveListHook* hook;
public:
   typedef std::bidirectional_iterator_tag iterator_category;
   typedef T value_type;
   typedef ptrdiff_t difference_type;
   typedef T* pointer;
   typedef T& reference;
   IntrusiveListIterator(IntrusiveListHook* hookPtr) : hook(hookPtr) {}
   IntrusiveListIterator() : hook(nullptr) {}
   IntrusiveListIterator(IntrusiveListIterator const& rhs) : hook(rhs.hook) {}

   IntrusiveListIterator& operator =(IntrusiveListIterator const& rhs)
   {
      hook = rhs.hook;
      return *this;
   }
   reference operator*() const
   {
      return *asPointer();
   }
   pointer operator->() const
   {
      return asPointer();
   }
   IntrusiveListIterator& operator++()
   {
      hook = hook->next;
      return *this;
   }
   T* asPointer() const
   {
      return parentFromMember<T>(hook, hookMember);
   }
   IntrusiveListIterator operator++(int)
   {
      IntrusiveListIterator temp(*this);
      hook = hook->next;
      return temp;
   }
   IntrusiveListIterator& operator--()
   {
      hook = hook->prev;
      return *this;
   }
   IntrusiveListIterator operator--(int)
   {
      IntrusiveListIterator temp(*this);
      hook = hook->prev;
      return temp;
   }
   bool operator==(IntrusiveListIterator const& rhs) const
   {
      return hook == rhs.hook;
   }
   bool operator!=(IntrusiveListIterator const& rhs) const
   {
      return hook != rhs.hook;
   }
};



template <typename T, IntrusiveListHook T::*hookMember>
class IntrusiveListConstIterator
{
   IntrusiveListHook* hook;
public:
   typedef std::bidirectional_iterator_tag iterator_category;
   typedef const T value_type;
   typedef ptrdiff_t difference_type;
   typedef const T* pointer;
   typedef const T& reference;
   IntrusiveListConstIterator(IntrusiveListHook * hookPtr) : hook(hookPtr) {}
   IntrusiveListConstIterator() : hook(nullptr) {}
   IntrusiveListConstIterator(IntrusiveListConstIterator const& rhs) : hook(rhs.hook) {}

   IntrusiveListConstIterator& operator =(IntrusiveListConstIterator const& rhs)
   {
      hook = rhs.hook;
      return *this;
   }
   reference operator*() const
   {
      return *asPointer();
   }
   pointer operator->() const
   {
      return asPointer();
   }
   IntrusiveListConstIterator& operator++()
   {
      hook = hook->next;
      return *this;
   }
   pointer asPointer() const
   {
      return parentFromMember<T>(hook, hookMember);
   }
   IntrusiveListConstIterator operator++(int)
   {
      IntrusiveListConstIterator temp(*this);
      hook = hook->next;
      return temp;
   }
   IntrusiveListConstIterator& operator--()
   {
      hook = hook->prev;
      return *this;
   }
   IntrusiveListConstIterator operator--(int)
   {
      IntrusiveListConstIterator temp(*this);
      hook = hook->prev;
      return temp;
   }
   bool operator==(IntrusiveListConstIterator const& rhs) const
   {
      return hook == rhs.hook;
   }
   bool operator!=(IntrusiveListConstIterator const& rhs) const
   {
      return hook != rhs.hook;
   }
};

template <typename T, IntrusiveListHook T::*hookMember>
IntrusiveListIterator<T, hookMember> asIterator(T* obj)
{
   return IntrusiveListIterator<T, hookMember>(obj);
}

template <typename T, IntrusiveListHook T::*hookMember>
class IntrusiveList
{
   IntrusiveListHook dummyFirst, dummyLast;
   void insertBefore(IntrusiveListHook* hookPtr, T* newNode)
   {
      auto* newHook = &(newNode->*hookMember);
      newHook->unlink();
      newHook->next = hookPtr;
      newHook->prev = hookPtr->prev;
      if (hookPtr->prev)
      {
         hookPtr->prev->next = newHook;
      }
      hookPtr->prev = newHook;
   }
   void insert(IntrusiveListHook* hookPtr, T* newNode)
   {
      auto* newHook = &(newNode->*hookMember);
      newHook->unlink();
      newHook->prev = hookPtr;
      newHook->next = hookPtr->next;
      if (hookPtr->next)
      {
         hookPtr->next->prev = newHook;
      }
      hookPtr->next = newHook;
   }
   void relinkRhs(IntrusiveList& rhs)
   {
      if (!rhs.empty())
      {
         dummyFirst.next = rhs.dummyFirst.next;
         dummyLast.prev = rhs.dummyLast.prev;

         dummyFirst.next->prev = &dummyFirst;
         dummyLast.prev->next = &dummyLast;

         rhs.dummyFirst.next = &rhs.dummyLast;
         rhs.dummyLast.prev = &rhs.dummyFirst;
      }
      else
      {
         dummyFirst.next = &dummyLast;
         dummyLast.prev = &dummyFirst;
      }
   }
   IntrusiveList(IntrusiveList const& rhs);
   IntrusiveList& operator=(IntrusiveList const& rhs);
public:
   typedef IntrusiveListIterator<T, hookMember> iterator;
   typedef IntrusiveListConstIterator<T, hookMember> const_iterator;
   typedef std::reverse_iterator<iterator> reverse_iterator;
   typedef std::reverse_iterator<const_iterator> reverse_const_iterator;
   IntrusiveList()
   {
      dummyFirst.next = &dummyLast;
      dummyLast.prev = &dummyFirst;
   }
   IntrusiveList(IntrusiveList&& rhs)
   {
      relinkRhs(rhs);
   }
   IntrusiveList& operator=(IntrusiveList&& rhs)
   {
      unlinkAll();
      relinkRhs(rhs);
      return *this;
   }
   void unlinkAll()
   {
      while (!empty())
      {
         dummyFirst.next->unlink();
      }
   }
   ~IntrusiveList()
   {
      unlinkAll();
   }
   void push_back(T* newNode)
   {
      insertBefore(&dummyLast, newNode);
   }
   void push_front(T* newNode)
   {
      insert(&dummyFirst, newNode);
   }
   T& front()
   {
      return *parentFromMember<T>(dummyFirst.next, hookMember);
   }
   T& back()
   {
      return *parentFromMember<T>(dummyLast.prev, hookMember);
   }
   void pop_front()
   {
      erase(&front());
   }
   void erase(T* obj)
   {
      (obj->*hookMember).unlink();
   }
   iterator begin()
   {
      return iterator(dummyFirst.next);
   }
   iterator end()
   {
      return iterator(&dummyLast);
   }
   reverse_iterator rbegin()
   {
      return reverse_iterator(end());
   }
   reverse_iterator rend()
   {
      return reverse_iterator(begin());
   }
   const_iterator begin() const
   {
      return const_iterator(dummyFirst.next);
   }
   const_iterator  end()  const
   {
      return const_iterator((IntrusiveListHook*)&dummyLast);
   }
   bool empty() const
   {
      return dummyFirst.next == &dummyLast;
   }
};


//double-buffer so that mid-stream deletes are okay.
template <typename T, IntrusiveListHook T::*hookMember, typename Func>
void safeIterate(IntrusiveList<T, hookMember>& intrusiveLinkedList, Func&& fun)
{
   IntrusiveList<T, hookMember> temp;
   while (!intrusiveLinkedList.empty())
   {
      auto& val = intrusiveLinkedList.front();
      (val.*hookMember).unlink();
      temp.push_back(&val);
      fun(val);
   }
   std::swap(intrusiveLinkedList, temp);
}
