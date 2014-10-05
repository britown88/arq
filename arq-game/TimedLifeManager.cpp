#include "ArqManagers.h"
#include "ArqComponents.h"
#include "engine\Application.h"
#include "engine\IOCContainer.h"

REGISTER_COMPONENT(TimedLifeComponent);

struct TTimedLifeComponent : public Component
{
   double startTime, length;
   TTimedLifeComponent(){}
   TTimedLifeComponent(double start, double length):startTime(start), length(length){}
};

REGISTER_COMPONENT(TTimedLifeComponent);

class TimedLifeManagerImpl : public Manager<TimedLifeManagerImpl, TimedLifeManager>
{
public:
   static void registerComponentCallbacks(Manager<TimedLifeManagerImpl, TimedLifeManager> &m)
   {
      m.add<TimedLifeComponent>();
   }

   void onChanged(Entity *e, const TimedLifeComponent &oldData, const TimedLifeComponent &newData, int key){e->lock<TTimedLifeComponent>()->length = newData.seconds;}
   void onAdded(Entity *e, const TimedLifeComponent &comp, int key)
   {
      auto &app = IOC.resolve<Application>();
      e->add(TTimedLifeComponent(app->getTime(), comp.seconds));
   }
   void onRemoved(Entity *e, const TimedLifeComponent &comp, int key){e->remove<TTimedLifeComponent>();}

   void onDelete(Entity *e)
   {
      if(auto cc = e->get<TimedLifeComponent>())
         e->remove<TTimedLifeComponent>();            
   }

   void update()
   {
      auto time = IOC.resolve<Application>()->getTime();
      std::vector<Entity*> toDelete;
      for(auto &comp : m_system->getComponentVector<TTimedLifeComponent>())
         if(time - comp.startTime >= comp.length)
            toDelete.push_back(comp.parent);

      for(auto e : toDelete)
         m_system->deleteEntity(e);
   }
};

std::unique_ptr<TimedLifeManager> buildTimedLifeManager()
{
   return std::unique_ptr<TimedLifeManager>(new TimedLifeManagerImpl());
}