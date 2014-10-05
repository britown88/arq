#include "ArqManagers.h"
#include "ArqComponents.h"
#include "ActorManager.h"
#include "engine\CoreComponents.h"

REGISTER_COMPONENT(AttackComponent);

class AttackManagerImpl : public Manager<AttackManagerImpl, AttackManager>
{
public:
   static void registerComponentCallbacks(Manager<AttackManagerImpl, AttackManager> &m)
   {
   }

   void onNew(Entity *e)
   {
      if(auto ac = e->get<AttackComponent>())
      {
         m_system->getManager<ActorManager>()->face(ac->owner, ac->dir);

         if(auto pos = e->lock<PositionComponent>())
         if(auto opos = ac->owner->get<PositionComponent>())
            pos->pos = opos->pos;

         if(ac->stunUser)
         {
            if(auto ovc = ac->owner->lock<VelocityComponent>())
               ovc->velocity = Float2();

            m_system->getManager<ActorManager>()->stun(ac->owner);
         }
      }
   }

   void onDelete(Entity *e)
   {
      if(auto ac = e->get<AttackComponent>())
      {
         if(ac->stunUser)
         {
            m_system->getManager<ActorManager>()->unstun(ac->owner);
         }
      }
   }


   void update()
   {
   }
};

std::unique_ptr<AttackManager> buildAttackManager()
{
   return std::unique_ptr<AttackManager>(new AttackManagerImpl());
}