#pragma once
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>

#include "Component.h"
#include "TypeInfo.h"
#include "boost\optional.hpp"

class Entity;
class EntitySystem;

static const size_t missingComponent = (size_t)-1;

class IManager
{
public:
   virtual ~IManager(){}
   virtual void onNew(Entity *e)=0;
   virtual void onDelete(Entity *e)=0;
   virtual void onComponentChanged(Entity *e, void* oldData, void* newData, int componentType, int key)=0;
   virtual void onComponentAdded(Entity *e, void *comp, int componentType, int key)=0;
   virtual void onComponentRemoved(Entity *e, void *comp, int componentType, int key)=0;
   
   virtual const std::vector<int> &listensForChangesOfType()=0;
   virtual void setSystem(EntitySystem *system)=0;
   virtual int getIndex()=0;//should be unique per instance
};

template<typename T>
struct ManagerIndices
{
   static int &getIndex()
   {
      static int index = 0;
      return index;
   }
};

typedef std::function<void (Entity*, void*, void*, int)> ComponentChangeCallback;
typedef std::function<void (Entity*, void*, int)> ComponentAddRemoveCallback;

template<typename CRTP, typename ManagingInterface=IManager>
class Manager : public ManagingInterface
{   
   std::vector<int> m_types;
   std::vector<ComponentChangeCallback> m_changeCallbacks;   
   std::vector<ComponentAddRemoveCallback> m_addCallbacks, m_removeCallbacks;  

protected:
   EntitySystem *m_system;
   const int m_index;

public:
   Manager():m_index(ManagerIndices<CRTP>::getIndex()++)
   {
      m_changeCallbacks.resize(componentCount);
      m_addCallbacks.resize(componentCount);
      m_removeCallbacks.resize(componentCount);
      CRTP::registerComponentCallbacks(*this);
   }
   virtual ~Manager(){}
   virtual void onNew(Entity *e){}
   virtual void onDelete(Entity *e){}
   int getIndex(){return m_index;}

   void setSystem(EntitySystem *system){m_system = system;}
   const std::vector<int> &listensForChangesOfType(){return m_types;}

   template <typename T>
   void add()
   {
      m_types.push_back(ComponentData<T>::getIndex());

      m_changeCallbacks[m_types.back()] = [&](Entity* e, void* oldData, void* newData, int key)
      {
         static_cast<CRTP*>(this)->onChanged(e, *(T*)oldData, *(T*)newData, key);
      };

      m_addCallbacks[m_types.back()] = [&](Entity* e, void* comp, int key)
      {
         static_cast<CRTP*>(this)->onAdded(e, *(T*)comp, key);
      };

      m_removeCallbacks[m_types.back()] = [&](Entity* e, void* comp, int key)
      {
         static_cast<CRTP*>(this)->onRemoved(e, *(T*)comp, key);
      };
   }

   void onComponentChanged(Entity* e, void* oldData, void* newData, int componentType, int key)
   {
      if (auto& cp = m_changeCallbacks[componentType]) 
         cp(e, oldData, newData, key);
   }
   void onComponentAdded(Entity *e, void *comp, int componentType, int key)
   {
      if (auto& cp = m_addCallbacks[componentType]) 
         cp(e, comp, key);
   }
   void onComponentRemoved(Entity *e, void *comp, int componentType, int key)
   {
      if (auto& cp = m_removeCallbacks[componentType]) 
         cp(e, comp, key);
   }
   
};

class EntitySystem
{
   std::vector<void*> m_componentLists;
   std::unordered_map<size_t, std::unique_ptr<IManager>> m_managers;
   std::vector<Entity*> m_entities;
   std::vector<size_t> m_openIndices;
   std::vector<std::vector<IManager*> > m_managerCallbackLookup;

   IManager *_addManager(size_t ID, std::unique_ptr<IManager> man);
   
public:
   EntitySystem();
   ~EntitySystem();

   template<typename T>
   T *addManager(std::unique_ptr<T> man)
   {
      auto out = static_cast<T*>(_addManager(TypeInfo<T>::instance().ID, std::move(man)));

      for(auto t : out->listensForChangesOfType())
         m_managerCallbackLookup[t].push_back(out);

      return out;
   }

   template<typename T>
   T *getManager()
   {
      auto iter = m_managers.find(TypeInfo<T>::instance().ID);
      if(iter != m_managers.end())
         return static_cast<T*>(iter->second.get());

      return nullptr;
   }

   void setComponentChanged(Entity *e, void* oldData, void* newData, int componentType, int key);
   void setComponentAdded(Entity *e, void* comp, int componentType, int key);
   void setComponentRemoved(Entity *e, void* comp, int componentType, int key);
   void setNew(Entity *e);
   void setDeleted(Entity *e);
   void setChanged(Entity *e);

   template<typename T>
   std::vector<T> &getComponentVector()
   {
      auto &handle = m_componentLists[ComponentData<T>::getIndex()];

      if(!handle)
         handle = new std::vector<T>();

      return *(std::vector<T>*)handle;
   }

   template<typename T>
   ComplexComponentList<T> &getComplexComponentList()
   {
      static_assert(std::is_base_of<ComplexComponent, T>::value, "Must be a Complex Component!");
      auto &handle = m_componentLists[ComponentData<T>::getIndex()];

      if(!handle)
         handle = new ComplexComponentList<T>();

      auto &cList = *(ComplexComponentList<T>*)handle;
      return cList;
   }

   template<typename T>
   std::vector<T> &getComponentVector(int index)
   {
      auto &cList = getComplexComponentList<T>();

      if(index >= cList.components.size())
         cList.components.resize(index + 1);

      return cList.components[index];
   }

   template<typename T>
   T &getComponent(size_t index)
   {
      return getComponentVector<T>()[index];      
   }


   template <typename T>
   T* deref(ComplexComponentList<T>& vec, ComponentLink const& link)
   {
      if(link.key == missingComponent)
         return (T*)&vec.lists[link.index];
      
      return &vec.components[link.key][link.index];
   }

   template<typename T>
   T *getComponent(size_t index, int subIndex)
   {
      auto& vec = getComplexComponentList<T>();
      auto &node = vec.lists[index];
      auto iter =  node.next;
      while(iter.key != missingComponent)
      {
         if(iter.key == subIndex)
            return deref(vec, iter);
         iter = deref(vec,iter)->next;
      }
      return nullptr;    
   }

   template<typename T>
   size_t addComponent(Entity *e, const T&comp)
   {
      auto &vec = getComponentVector<T>();
      vec.push_back(comp);

      vec.back().parent = e;

      return vec.size() - 1;
   }

   template<typename T>
   void addComponent(Entity *e, const T&comp, int index)
   {
      auto &list = getComplexComponentList<T>();      

      auto &cIndex = e->getComplexIndex<T>();
      if(cIndex == missingComponent)
      {
         list.lists.push_back(ComponentListDummy<T>());
         cIndex = list.lists.size() - 1;
         list.lists.back().parent = e;
         ComponentLink missing;
         missing.key = missingComponent;
         missing.index = cIndex;
         list.lists.back().next = list.lists.back().prev = missing;
      }

      if(index >= list.components.size())
         list.components.resize(index + 1);

      list.components[index].push_back(comp);

      ComponentLink base;
      base.key = missingComponent;
      base.index = cIndex;
      auto &c = list.components[index].back();
      c.prev = base;
      c.next = list.lists[cIndex].next;
      c.key = index;

      list.lists[cIndex].next.key = index;
      list.lists[cIndex].next.index = list.components[index].size()-1;
      c.parent = e;
      deref(list, c.next)->prev = list.lists[cIndex].next;
   }

   void removeComponent(size_t type, size_t index);
   void removeComponent(size_t type, size_t index, int subIndex);
   Entity *createEntity();
   void deleteEntity(Entity *e);
};

template<typename T>
class LockedComponent
{
   Entity* e;
   T newComponent;
   T const& oldComponent;
   int key;

   typedef void (LockedComponent::*undefined_bool)() const;
   void this_type_does_not_support_comparisons() const {}
public:
   LockedComponent(T const& old, int key_in = 0) 
      : newComponent(old), oldComponent(old),
      e(old.parent), key(key_in)
   {
   }
   LockedComponent(LockedComponent && rhs)
      :newComponent(rhs.newComponent), oldComponent(rhs.oldComponent), e(rhs.e), key(rhs.key)
   {
      rhs.e = nullptr;
   }
   LockedComponent()
      :e(nullptr), oldComponent(newComponent)
   {}

   operator undefined_bool()
   {
      if(e)
         return &LockedComponent::this_type_does_not_support_comparisons;
      else
         return nullptr;
   }

   ~LockedComponent()
   {
      if(!e)
         return;

      std::swap((T&)newComponent, (T&)oldComponent);

      e->getSystem()->setComponentChanged(e, 
         &newComponent, 
         &(T&)oldComponent, 
         ComponentData<T>::getIndex(),
         key);

      //std::swap((T&)newComponent, (T&)oldComponent);
      //e->add(newComponent);
   }

   T* operator->() {return &newComponent;}

  
};

class Entity
{
   std::vector<size_t> m_components;
   EntitySystem *m_system;

       
public:
   Entity(EntitySystem *system)
      :m_system(system)
   {
      m_components.resize(componentCount, missingComponent);
   }

   ~Entity()
   {
      for(size_t i = 0; i < componentCount; ++i)
         if(m_components[i] != missingComponent)
            m_system->removeComponent(i, m_components[i]);
   }

   EntitySystem *getSystem(){return m_system;}
   void setNew() {m_system->setNew(this);}
   void setChanged() {m_system->setChanged(this);}
   void setDeleted() {m_system->setDeleted(this);}

   template<typename T>
   void add(const T &comp)
   {
      int type = ComponentData<T>::getIndex();      
      
      m_components[type] = m_system->addComponent(this, comp);
      m_system->setComponentAdded(this, (void*)&comp, type, 0);
   }

   template<typename T>
   void add(const T &comp, int index)
   {
      int type = ComponentData<T>::getIndex();      
      
      m_system->addComponent(this, comp, index);
      m_system->setComponentAdded(this, (void*)&comp, type, index);
   }

   template<typename T>
   void remove()
   {
      size_t type = ComponentData<T>::getIndex();
      size_t &index = m_components[type];

      m_system->setComponentRemoved(this, &m_system->getComponent<T>(index), type, 0);

      if(index != missingComponent)
         m_system->removeComponent(type, index);

      index = missingComponent;
   }

   template<typename T>
   void remove(int subIndex)
   {
      size_t type = ComponentData<T>::getIndex();
      size_t &index = m_components[type];

      m_system->setComponentRemoved(this, m_system->getComponent<T>(index, subIndex), type, subIndex);

      if(index != missingComponent)
         m_system->removeComponent(type, index, subIndex);
   }

   template<typename T>
   const T *get()
   {
      size_t index = m_components[ComponentData<T>::getIndex()];

      if(index == missingComponent)
         return nullptr;

      return &m_system->getComponent<T>(index);
   }

   template<typename T>
   const T *get(int subIndex)
   {
      size_t index = m_components[ComponentData<T>::getIndex()];

      if(index == missingComponent)
         return nullptr;

      return m_system->getComponent<T>(index, subIndex);
   }

   template<typename T>
   LockedComponent<T> lock()
   {
      size_t index = m_components[ComponentData<T>::getIndex()];

      if(index == missingComponent)
         return LockedComponent<T>();

      return LockedComponent<T>(m_system->getComponent<T>(index));
   }

   template<typename T>
   LockedComponent<T> lock(int subIndex)
   {
      auto comp = get<T>(subIndex);

      if(!comp)
         return LockedComponent<T>();

      return LockedComponent<T>(*comp, subIndex);
   }

   template<typename T>
   size_t &getComplexIndex()
   {
      return m_components[ComponentData<T>::getIndex()];
   }

   void moveComponent(size_t type, size_t index)
   {
      m_components[type] = index;
   }

   void eraseComplexComponent(size_t type)
   {
      m_components[type] = missingComponent;
   }
};