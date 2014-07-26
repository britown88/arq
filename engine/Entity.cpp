#include "Entity.h"
#include "CoreComponents.h"

EntitySystem::EntitySystem()
{
   registerCoreComponents();
   m_componentLists.resize(componentCount, nullptr);
   m_managerCallbackLookup.resize(componentCount);
}
EntitySystem::~EntitySystem()
{
   for(size_t &index : m_openIndices)
   {
      auto &e = m_entities[index];
      free(e);
      e = nullptr;
   }

   for(auto &e : m_entities)
   {
      if(e)
      {
         e->~Entity();
         free(e);
      }
   }

   for(size_t i = 0; i < componentCount; ++i)
      getVTable(i)->deleteList(m_componentLists[i]);
}

IManager *EntitySystem::_addManager(size_t ID, std::unique_ptr<IManager> man)
{
   man->setSystem(this);
   m_managers.insert(std::make_pair(ID, std::move(man)));
   return m_managers.at(ID).get();
}

void EntitySystem::setComponentChanged(Entity *e, void* oldData, void* newData, int componentType, int key)
{
   for(auto m : m_managerCallbackLookup[componentType]) 
      m->onComponentChanged(e, oldData, newData, componentType, key);
}

void EntitySystem::setComponentAdded(Entity *e, void* comp, int componentType, int key)
{
   for(auto m : m_managerCallbackLookup[componentType]) 
      m->onComponentAdded(e, comp, componentType, key);
}

void EntitySystem::setComponentRemoved(Entity *e, void* comp, int componentType, int key)
{
   for(auto m : m_managerCallbackLookup[componentType]) 
      m->onComponentRemoved(e, comp, componentType, key);
}

void EntitySystem::setNew(Entity *e)
{
   for(auto &man : m_managers)
      man.second->onNew(e);
}

void EntitySystem::setDeleted(Entity *e)
{
   for(auto &man : m_managers)
      man.second->onDelete(e);
}

void EntitySystem::setChanged(Entity *e)
{
   setDeleted(e);
   setNew(e);
}

void EntitySystem::removeComponent(size_t type, size_t index)
{
   getVTable(type)->deleteAt(m_componentLists[type], index);
}

void EntitySystem::removeComponent(size_t type, size_t index, int subIndex)
{
   getVTable(type)->deleteAtComplex(m_componentLists[type], index, subIndex);
}

Entity *EntitySystem::createEntity()
{
   if(m_openIndices.empty())
   {
      auto e = (Entity*)malloc(sizeof(Entity) + sizeof(int));
      new(e) Entity(this);

      //dont try this at home, kids
      auto &index = *(int*)(e + 1);
      index = m_entities.size();

      m_entities.push_back(e);
      return m_entities.back();
   }

   auto &e = m_entities[m_openIndices.back()];
   m_openIndices.pop_back();

   new(e) Entity(this);
   return e;
}

void EntitySystem::deleteEntity(Entity *e)
{
   setDeleted(e);

   auto &index = *(int*)(e + 1);
   m_openIndices.push_back(index);
   e->~Entity();
}