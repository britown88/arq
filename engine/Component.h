#pragma once

#include <type_traits>
#include <deque>
#include <vector>

class ComponentTypeVTable
{
public:
   virtual void deleteList(void *list)=0;
   virtual void deleteAt(void *list, size_t index)=0;
   virtual void deleteAtComplex(void *list, size_t index, size_t subIndex)=0;
};

extern std::vector<ComponentTypeVTable*> vTables;

//register different components into the system and they
//all get a unique index :D <3 xD
template<typename T>
struct ComponentData
{
   static size_t &getIndex()
   {
      static size_t index = (size_t)-1;
      return index;
   }
};

extern size_t componentCount;

class Entity;

class Component
{
public:
   Entity *parent;
};
struct ComponentLink
{
   size_t key, index;
};

class ComplexComponent
{
public:
   ComponentLink prev, next;
   Entity *parent;
   int key;
};

template<typename T>
struct ComponentListDummy
{
   ComponentLink prev, next;
   Entity *parent;
};

template<typename T>
class ComplexComponentList
{
public:
   std::deque<std::vector<T>> components;
   std::vector<ComponentListDummy<T>> lists;
};


void registerVTable(ComponentTypeVTable *vTable);
ComponentTypeVTable *getVTable(size_t index);

template<typename T>
ComponentTypeVTable *getComponentVTable()
{
   return getVTable(ComponentData<T>::getIndex());
}

template<typename T>
void registerComponent(std::integral_constant<bool, false>)
{
   class VTableImpl : public ComponentTypeVTable
   {
   public:
      void deleteList(void *list)
      {
         delete (std::vector<T>*)list;
      }

      void deleteAt(void *list, size_t index)
      {
         auto &vec = *(std::vector<T>*)list;
         int vecSize = vec.size();

         if(index != vecSize - 1)
         {
            vec[index] = std::move(vec.back());
            vec[index].parent->moveComponent(ComponentData<T>::getIndex(), index);
         }

         vec.pop_back();
      }

      void deleteAtComplex(void *list, size_t index, size_t subIndex)
      {
         throw std::exception("Trying to delete complex from a non-complex vtable!");
      }
   };

   static VTableImpl vTable;
   registerVTable(&vTable);
}

template<typename T>
void registerComponent(std::integral_constant<bool, true>)
{
   class VTableImpl : public ComponentTypeVTable
   {
   public:
      void deleteList(void *list)
      {
         delete (ComplexComponentList<T>*)list;
      }

      void deleteAt(void *list, size_t index)
      {
         //throw std::exception("Trying to delete non-complex from a complex vtable!");
         auto &vec = *(ComplexComponentList<T>*)list;
         auto &node = vec.lists[index];

         auto iter = node.next;
         while(iter.key != missingComponent)
         {
            auto sub = iter.key;
            iter = deref(vec, iter).next;
            deleteAtComplex(list, index, sub);
         }
      }

      T& deref(ComplexComponentList<T>& vec, ComponentLink const& link)
      {
         if(link.key == missingComponent)
            return *(T*)&vec.lists[link.index];

         return vec.components[link.key][link.index];
      }
      void deleteAtComplex(void *list, size_t index, size_t subIndex)
      {
         auto &vec = *(ComplexComponentList<T>*)list;
         auto &node = vec.lists[index];

         auto iter = node.next;
         while(iter.key != missingComponent)
         {
            if(iter.key == subIndex)
            {
               auto& val = deref(vec, iter);
               deref(vec, val.next).prev = val.prev;
               deref(vec, val.prev).next = val.next;

               auto &subVec = vec.components[subIndex];
               auto internalIndex = iter.index;
               int vecSize = subVec.size();

               if(internalIndex != vecSize - 1)
               {
                  auto oldPrev = &deref(vec, subVec.back().prev);
                  auto oldNext = &deref(vec, subVec.back().next);

                  ComponentLink base;
                  base.key = subIndex;
                  base.index = internalIndex;

                  oldPrev->next = base;
                  oldNext->prev = base;

                  subVec[internalIndex] = std::move(subVec.back());                  
               }

               subVec.pop_back();
               break;
            }
            iter = deref(vec, iter).next;
         }  

         if(node.next.key == missingComponent)
         {
            node.parent->eraseComplexComponent(ComponentData<T>::getIndex());
            int vecSize = vec.lists.size();

            if(index != vecSize - 1)
            {
               auto oldPrev = &deref(vec, vec.lists.back().prev);
               auto oldNext = &deref(vec, vec.lists.back().next);


               ComponentLink base;
               base.key = missingComponent;
               base.index = index;

               oldPrev->next = base;
               oldNext->prev = base;

               vec.lists[index] = std::move(vec.lists.back());
               vec.lists[index].parent->moveComponent(ComponentData<T>::getIndex(), index);
            }               

            vec.lists.pop_back();
         }
      }
   };

   static VTableImpl vTable;
   registerVTable(&vTable);
}

template<typename T>
void registerComponent()
{
   ComponentData<T>::getIndex() = componentCount++;

   registerComponent<T>(typename std::is_base_of<ComplexComponent, T>::type());
}


template<typename T>
class ComponentRegistrationFactory
{
public:
   ComponentRegistrationFactory()
   {
      registerComponent<T>();
   }
};

#pragma region DONT_LOOK_AT_ME
#define CONCAT2(x,y) x##y
#define CONCAT(x,y) CONCAT2(x,y)
#define REGISTER_COMPONENT(...) namespace { ComponentRegistrationFactory<__VA_ARGS__> CONCAT(_register,__LINE__); }

#pragma endregion
