#include "engine\Component.h"
#include "engine\StateMachine.h"
#include "engine\CoreComponents.h"
#include "engine\ComponentHelpers.h"
#include "engine\GameMath.h"
#include "engine\IOCContainer.h"
#include "engine\Application.h"

#include "ActorManager.h"
#include "GridManager.h"
#include "ArqComponents.h"
#include "PartitionManager.h"
#include "GameData.h"
#include "engine\Logs.h"

#include "boost\optional.hpp"

REGISTER_COMPONENT(ActorComponent);

class ActorState
{
protected:
   EntitySystem *m_system;
   Entity *e;
   
   const Application &app;
public:
   ActorState(EntitySystem *system, Entity *e)
      :m_system(system), app(*IOC.resolve<Application>()), e(e){}
   virtual void onEnter(){}
   virtual void onLeave(){}

   virtual void update(){}

   virtual void moveLeft(){}
   virtual void moveRight(){}
   virtual void moveUp(){}
   virtual void moveDown(){}
   
   virtual void stopLeft(){}
   virtual void stopRight(){}
   virtual void stopUp(){}
   virtual void stopDown(){}
};
typedef StateMachine<ActorState> CharSM;

struct TActorComponent : public Component
{
   mutable std::shared_ptr<CharSM> sm;

   enum class MovementType : unsigned int
   {
      Up = 0,
      Down,
      Left,
      Right
   };

   mutable unsigned char moveFlags[4];

   TActorComponent()
      :sm(std::make_shared<CharSM>())
   {
      memset(moveFlags, 0, sizeof(moveFlags));
   }
};

REGISTER_COMPONENT(TActorComponent);

const static float MoveSpeed = 3.0f;

#pragma region Actor functions

void characterMove(Entity *e, TActorComponent::MovementType type)
{
   if(auto cc = e->get<TActorComponent>())
      cc->moveFlags[(unsigned int)type] = 1;
}

void characterStop(Entity *e, TActorComponent::MovementType type)
{
   if(auto cc = e->get<TActorComponent>())
      cc->moveFlags[(unsigned int)type] = 0;
}

void characterSetState(Entity *e, ActorState *state)
{
   if(auto cc = e->get<TActorComponent>())
      cc->sm->set(state);
}
//returns whether offset succeeded
bool characterOffsetPosition(Entity *e, EntitySystem *system, float x, float y)
{
   auto ebox = ComponentHelpers::getCollisionBox(e);
   ebox.offset(Float2(x, y));

   auto gridCollisions = system->getManager<GridManager>()->collisionAt(ebox);
   if(gridCollisions.empty())
   {
      if(auto pc = e->lock<PositionComponent>())
      {
         pc->pos.x += x;
         pc->pos.y += y;
      }

      return true;
   }

   return false;
}
void characterUpdateGroundMovement(Entity *e)
{
   float dt = IOC.resolve<Application>()->dt();

   if(auto vc = e->lock<VelocityComponent>())
   if(auto cc = e->get<TActorComponent>())
   {
      typedef TActorComponent::MovementType mt;

      if(cc->moveFlags[(unsigned int)mt::Up])
         vc->velocity.y = -MoveSpeed;
      else if(cc->moveFlags[(unsigned int)mt::Down])
         vc->velocity.y = MoveSpeed;
      else
         vc->velocity.y = 0;

      if(cc->moveFlags[(unsigned int)mt::Left])
         vc->velocity.x = -MoveSpeed;
      else if(cc->moveFlags[(unsigned int)mt::Right])
         vc->velocity.x = MoveSpeed;
      else
         vc->velocity.x = 0;
   }
}

void characterUpdateGroundSprite(Entity *e)
{
   if(auto tcc = e->get<TActorComponent>())
   if(auto cc = e->get<ActorComponent>())
   if(auto sc = e->lock<SpriteComponent>())
   {
      typedef TActorComponent::MovementType mt;

      bool up = tcc->moveFlags[(unsigned int)mt::Up];
      bool down = tcc->moveFlags[(unsigned int)mt::Down];
      bool left = tcc->moveFlags[(unsigned int)mt::Left];
      bool right = tcc->moveFlags[(unsigned int)mt::Right];

      if((up && sc->sprite == cc->upRunSprite) || 
         (down && sc->sprite == cc->downRunSprite) || 
         (left && sc->sprite == cc->leftRunSprite) || 
         (right && sc->sprite == cc->rightRunSprite))
         return;

      if(up) sc->sprite = cc->upRunSprite;
      else if(down) sc->sprite = cc->downRunSprite;
      else if(left) sc->sprite = cc->leftRunSprite;
      else if(right) sc->sprite = cc->rightRunSprite;

   }
}

#pragma endregion

class ActorManagerImpl : public Manager<ActorManagerImpl, ActorManager>
{
   std::vector<Entity*> m_players;
public:
   ActorManagerImpl()
   {
      m_players.resize(GameData::PlayerCount, nullptr);
   }

   static void registerComponentCallbacks(Manager<ActorManagerImpl, ActorManager> &m)
   {
      
   }
   void onNew(Entity *e)
   {
      if(auto cc = e->get<ActorComponent>())
         if(cc->playerNumber < GameData::PlayerCount)
         {
            m_players[cc->playerNumber] = cc->parent;
            m_players[cc->playerNumber]->add(TActorComponent());
            m_players[cc->playerNumber]->get<TActorComponent>()->sm->set(buildGroundState(m_players[cc->playerNumber]));
         } 
   }
   void onDelete(Entity *e)
   {
      if(auto cc = e->get<ActorComponent>())
         if(cc->playerNumber < GameData::PlayerCount && m_players[cc->playerNumber] == cc->parent)
         {
            m_players[cc->playerNumber]->remove<TActorComponent>();
            m_players[cc->playerNumber] = nullptr;
         }
            
   }
   Entity *getPlayer(int playerNumber)
   {
      if(playerNumber >= 0 && playerNumber < GameData::PlayerCount)
         return m_players[playerNumber];

      return nullptr;
   }

   ActorState *buildGroundState(Entity *e)
   {
      class GroundState : public ActorState
      {
         ActorManagerImpl *m_manager;
      public:
         GroundState(ActorManagerImpl *manager, EntitySystem *system, Entity *e)
            :m_manager(manager), ActorState(system, e){}

         virtual void update()
         {          
            
            characterUpdateGroundMovement(e);
            characterUpdateGroundSprite(e);
         }

         virtual void moveLeft(){characterMove(e, TActorComponent::MovementType::Left);}
         virtual void moveRight(){characterMove(e, TActorComponent::MovementType::Right);}
         virtual void moveUp(){characterMove(e, TActorComponent::MovementType::Up);}
         virtual void moveDown(){characterMove(e, TActorComponent::MovementType::Down);}

         virtual void stopLeft(){characterStop(e, TActorComponent::MovementType::Left);}
         virtual void stopRight(){characterStop(e, TActorComponent::MovementType::Right);}
         virtual void stopUp(){characterStop(e, TActorComponent::MovementType::Up);}
         virtual void stopDown(){characterStop(e, TActorComponent::MovementType::Down);}
      };

      return new GroundState(this, m_system, e);
   }

   void update()   
   {
      for (auto comp : m_system->getComponentVector<TActorComponent>())
      {
         if(auto cc = comp.parent->get<TActorComponent>())
         if(auto &sm = *cc->sm)
         {
            sm->update();
         }
      }
   }

   template<typename Func>
   void characterStateFunction(int player, Func && func)
   {
      if(auto e = getPlayer(player))
         if(auto cc = e->get<TActorComponent>())
            if(auto &sm = *cc->sm)
               func(sm);
   }

   void moveLeft(int player){characterStateFunction(player, [=](CharSM& sm){sm->moveLeft();});}
   void moveRight(int player){characterStateFunction(player, [=](CharSM& sm){sm->moveRight();});}
   void moveUp(int player){characterStateFunction(player, [=](CharSM& sm){sm->moveUp();});}
   void moveDown(int player){characterStateFunction(player, [=](CharSM& sm){sm->moveDown();});}

   void stopLeft(int player){characterStateFunction(player, [=](CharSM& sm){sm->stopLeft();});}
   void stopRight(int player){characterStateFunction(player, [=](CharSM& sm){sm->stopRight();});}
   void stopUp(int player){characterStateFunction(player, [=](CharSM& sm){sm->stopUp();});}
   void stopDown(int player){characterStateFunction(player, [=](CharSM& sm){sm->stopDown();});}
};

std::unique_ptr<ActorManager> buildActorManager()
{
   return std::unique_ptr<ActorManager>(new ActorManagerImpl());
}