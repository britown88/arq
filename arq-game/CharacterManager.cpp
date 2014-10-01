#include "engine\Component.h"
#include "engine\StateMachine.h"
#include "engine\CoreComponents.h"
#include "engine\ComponentHelpers.h"
#include "engine\GameMath.h"
#include "engine\IOCContainer.h"
#include "engine\Application.h"

#include "CharacterManager.h"
#include "GridManager.h"
#include "ArqComponents.h"
#include "PartitionManager.h"
#include "GameData.h"
#include "engine\Logs.h"

#include "boost\optional.hpp"

REGISTER_COMPONENT(CharacterComponent);

class CharacterState
{
protected:
   EntitySystem *m_system;
   Entity *e;
   
   const Application &app;
public:
   CharacterState(EntitySystem *system, Entity *e)
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
typedef StateMachine<CharacterState> CharSM;

struct TCharacterComponent : public Component
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

   TCharacterComponent()
      :sm(std::make_shared<CharSM>())
   {
      memset(moveFlags, 0, sizeof(moveFlags));
   }
};

REGISTER_COMPONENT(TCharacterComponent);

const static float MoveSpeed = 3.0f;

#pragma region Character functions

void characterMove(Entity *e, TCharacterComponent::MovementType type)
{
   if(auto cc = e->get<TCharacterComponent>())
      cc->moveFlags[(unsigned int)type] = 1;
}

void characterStop(Entity *e, TCharacterComponent::MovementType type)
{
   if(auto cc = e->get<TCharacterComponent>())
      cc->moveFlags[(unsigned int)type] = 0;
}

void characterSetState(Entity *e, CharacterState *state)
{
   if(auto cc = e->get<TCharacterComponent>())
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
   if(auto vc = e->lock<VelocityComponent>())
   if(auto cc = e->get<TCharacterComponent>())
   {
      typedef TCharacterComponent::MovementType mt;

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
   //if(auto tcc = e->get<TCharacterComponent>())
   //if(auto cc = e->get<CharacterComponent>())
   //if(auto sc = e->lock<SpriteComponent>())
   //{
   //   switch(tcc->movement)
   //   {
   //   case TCharacterComponent::MovementType::Left:
   //      if(sc->sprite != cc->runSprite)
   //         sc->sprite = cc->runSprite;
   //      break;
   //   case TCharacterComponent::MovementType::Right:
   //      if(sc->sprite != cc->runSprite)
   //         sc->sprite = cc->runSprite;
   //      break;
   //   case TCharacterComponent::MovementType::Stopped:
   //      if(sc->sprite != cc->idleSprite)
   //         sc->sprite = cc->idleSprite;
   //      break;
   //   }
   //}
}

#pragma endregion

class CharacterManagerImpl : public Manager<CharacterManagerImpl, CharacterManager>
{
   std::vector<Entity*> m_players;
public:
   CharacterManagerImpl()
   {
      m_players.resize(GameData::PlayerCount, nullptr);
   }

   static void registerComponentCallbacks(Manager<CharacterManagerImpl, CharacterManager> &m)
   {
      
   }
   void onNew(Entity *e)
   {
      if(auto cc = e->get<CharacterComponent>())
         if(cc->playerNumber < GameData::PlayerCount)
         {
            m_players[cc->playerNumber] = cc->parent;
            m_players[cc->playerNumber]->add(TCharacterComponent());
            m_players[cc->playerNumber]->get<TCharacterComponent>()->sm->set(buildGroundState(m_players[cc->playerNumber]));
         } 
   }
   void onDelete(Entity *e)
   {
      if(auto cc = e->get<CharacterComponent>())
         if(cc->playerNumber < GameData::PlayerCount && m_players[cc->playerNumber] == cc->parent)
         {
            m_players[cc->playerNumber]->remove<TCharacterComponent>();
            m_players[cc->playerNumber] = nullptr;
         }
            
   }
   Entity *getPlayer(int playerNumber)
   {
      if(playerNumber >= 0 && playerNumber < GameData::PlayerCount)
         return m_players[playerNumber];

      return nullptr;
   }

   CharacterState *buildGroundState(Entity *e)
   {
      class GroundState : public CharacterState
      {
         CharacterManagerImpl *m_manager;
      public:
         GroundState(CharacterManagerImpl *manager, EntitySystem *system, Entity *e)
            :m_manager(manager), CharacterState(system, e){}

         virtual void update()
         {          
            
            characterUpdateGroundMovement(e);
            characterUpdateGroundSprite(e);
         }

         virtual void moveLeft(){characterMove(e, TCharacterComponent::MovementType::Left);}
         virtual void moveRight(){characterMove(e, TCharacterComponent::MovementType::Right);}
         virtual void moveUp(){characterMove(e, TCharacterComponent::MovementType::Up);}
         virtual void moveDown(){characterMove(e, TCharacterComponent::MovementType::Down);}

         virtual void stopLeft(){characterStop(e, TCharacterComponent::MovementType::Left);}
         virtual void stopRight(){characterStop(e, TCharacterComponent::MovementType::Right);}
         virtual void stopUp(){characterStop(e, TCharacterComponent::MovementType::Up);}
         virtual void stopDown(){characterStop(e, TCharacterComponent::MovementType::Down);}
      };

      return new GroundState(this, m_system, e);
   }

   void update()   
   {
      for (auto comp : m_system->getComponentVector<TCharacterComponent>())
      {
         if(auto cc = comp.parent->get<TCharacterComponent>())
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
         if(auto cc = e->get<TCharacterComponent>())
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

std::unique_ptr<CharacterManager> buildCharacterManager()
{
   return std::unique_ptr<CharacterManager>(new CharacterManagerImpl());
}