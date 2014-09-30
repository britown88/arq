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
REGISTER_COMPONENT(CarriedComponent);
REGISTER_COMPONENT(TRemainingTimeComponent);

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
   virtual void resolveCollision(std::vector<Entity*> eList){}
   virtual void updateGravity(){}
   virtual void update(){}

   virtual void moveLeft(){}
   virtual void moveRight(){}
   virtual void stop(){}

   virtual void jump(){}
   virtual void endJump(){}
};
typedef StateMachine<CharacterState> CharSM;

struct TCharacterComponent : public Component
{
   mutable std::shared_ptr<CharSM> sm;

   mutable bool jumping;
   mutable float jumpStartY;

   enum class MovementType : unsigned int
   {
      Stopped = 0,
      Left,
      Right
   };

   mutable MovementType movement;
   mutable int carryingReferences;

   TCharacterComponent()
      :sm(std::make_shared<CharSM>()), jumping(false), jumpStartY(0.0f), movement(MovementType::Stopped){}
};

REGISTER_COMPONENT(TCharacterComponent);

struct TChildren : public Component
{
   mutable std::vector<Entity *> children;
};

REGISTER_COMPONENT(TChildren);

const static float Gravity = 0.5f;
const static float MoveSpeed = 3.0f;
const static float AirMoveAccel = 0.25f;
const static float JumpImpulse = 5.2f;
const static float JumpHeight = 100.0f;

#pragma region Character functions

void characterMoveRight(Entity *e)
{
   if(auto cc = e->get<TCharacterComponent>())
      cc->movement = TCharacterComponent::MovementType::Right;

   if(auto tc = e->lock<TextureComponent>())
      tc->flipX = false;
}

void characterMoveLeft(Entity *e)
{
   if(auto cc = e->get<TCharacterComponent>())
      cc->movement = TCharacterComponent::MovementType::Left;

   if(auto tc = e->lock<TextureComponent>())
      tc->flipX = true;
}

void characterStop(Entity *e)
{
   if(auto cc = e->get<TCharacterComponent>())
      cc->movement = TCharacterComponent::MovementType::Stopped;
}

bool characterJump(Entity *e)
{
   if(auto vc = e->lock<VelocityComponent>())
   if(auto pc = e->get<PositionComponent>())
   if(auto cc = e->get<TCharacterComponent>())
   {
      if(!cc->jumping)
      {
         cc->jumping = true;
         cc->jumpStartY = pc->pos.y;
         vc->velocity.y = -JumpImpulse;
         return true;
      }
   }

   return false;
}
void characterEndJump(Entity *e)
{
   if(auto cc = e->get<TCharacterComponent>())
   {
      if(cc->jumping)
         cc->jumping = false;
   }
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

void characterUpdateCarriedVelocity(Entity *e, EntitySystem *system)
{
   if(auto carried = e->get<CarriedComponent>())
   if(carried->carrier)
   {
      if(auto vc = e->lock<VelocityComponent>())
      if(auto cvc = carried->carrier->get<VelocityComponent>())
      {
         vc->velocity = cvc->velocity;
      }

      if(auto rtc = carried->carrier->get<TRemainingTimeComponent>())
      {
         if(rtc->remainingTime < 1.0f)
         {
            characterOffsetPosition(e, system, rtc->deltaPosition.x, rtc->deltaPosition.y);
            if(auto ertc = e->get<TRemainingTimeComponent>())
               ertc->remainingTime = rtc->remainingTime;
         }
            
      }
   }
   
}

//optionally returns a normal if a collision occured
boost::optional<Float2> characterCheckGridCollision(Entity *e, EntitySystem *system)
{
   boost::optional<Float2> out;

   if(auto rtc = e->get<TRemainingTimeComponent>())
   {
      Float2 n;
      float t = rtc->remainingTime;
      if(checkGridCollision(system, e, n, t))
         out = n;

      rtc->remainingTime = t;
   }

   return out;
}

void characterUpdateAirSprite(Entity *e)
{
   if(auto vc = e->get<VelocityComponent>())
   if(auto cc = e->get<CharacterComponent>())
   if(auto sc = e->lock<SpriteComponent>())
   {
      if(vc->velocity.y < 0.0f)
      {
         if(sc->sprite != cc->jumpUpSprite)
            sc->sprite = cc->jumpUpSprite;
      }
      else
      {
         if(sc->sprite != cc->jumpDownSprite)
            sc->sprite = cc->jumpDownSprite;
      }
   }
}

void characterUpdateAirMovement(Entity *e, double dt)
{
   if(auto vc = e->lock<VelocityComponent>())
   if(auto pc = e->get<PositionComponent>())
   if(auto cc = e->get<TCharacterComponent>())
   {
      switch(cc->movement)
      {
      case TCharacterComponent::MovementType::Left:
         vc->velocity.x = std::max(vc->velocity.x - (float)(AirMoveAccel * dt), -MoveSpeed);
         break;
      case TCharacterComponent::MovementType::Right:
         vc->velocity.x = std::min(vc->velocity.x + (float)(AirMoveAccel * dt), MoveSpeed);
         break;
      case TCharacterComponent::MovementType::Stopped:
         if(vc->velocity.x > 0.0f)
            vc->velocity.x = std::max(vc->velocity.x - (float)(AirMoveAccel * dt), 0.0f);
         else if(vc->velocity.x < 0.0f)
            vc->velocity.x = std::min(vc->velocity.x + (float)(AirMoveAccel * dt), 0.0f);
         break;
      }             
   }
}

void characterUpdateCarriedMovement(Entity *e)
{
   if(auto vc = e->lock<VelocityComponent>())
   if(auto tcc = e->get<TCharacterComponent>())
   {
      switch(tcc->movement)
      {
      case TCharacterComponent::MovementType::Left:
         vc->velocity.x -= MoveSpeed;
         break;
      case TCharacterComponent::MovementType::Right:
         vc->velocity.x += MoveSpeed;
         break;
      }
   }
}

void characterUpdateGroundMovement(Entity *e)
{
   if(auto vc = e->lock<VelocityComponent>())
   if(auto cc = e->get<TCharacterComponent>())
   {
      switch(cc->movement)
      {
      case TCharacterComponent::MovementType::Left:
         vc->velocity.x = -MoveSpeed;
         break;
      case TCharacterComponent::MovementType::Right:
         vc->velocity.x = MoveSpeed;
         break;
      case TCharacterComponent::MovementType::Stopped:
         vc->velocity.x = 0.0f;
         break;
      }
   }
}

void characterUpdateGravity(Entity *e, double dt)
{
   if(auto vc = e->lock<VelocityComponent>())
   if(auto pc = e->get<PositionComponent>())
   if(auto cc = e->get<TCharacterComponent>())
   {
      if(cc->jumping)
      {
         if((cc->jumpStartY - pc->pos.y) < JumpHeight)
         {
            vc->velocity.y = -JumpImpulse;
         }
         else
         {
            cc->jumping = false;
         }
      }
      else
         vc->velocity.y += Gravity * dt;   
   }
}

void characterUpdateGroundSprite(Entity *e)
{
   if(auto tcc = e->get<TCharacterComponent>())
   if(auto cc = e->get<CharacterComponent>())
   if(auto sc = e->lock<SpriteComponent>())
   {
      switch(tcc->movement)
      {
      case TCharacterComponent::MovementType::Left:
         if(sc->sprite != cc->runSprite)
            sc->sprite = cc->runSprite;
         break;
      case TCharacterComponent::MovementType::Right:
         if(sc->sprite != cc->runSprite)
            sc->sprite = cc->runSprite;
         break;
      case TCharacterComponent::MovementType::Stopped:
         if(sc->sprite != cc->idleSprite)
            sc->sprite = cc->idleSprite;
         break;
      }
   }
}

//returns whether on a grid ground
bool characterOnGround(Entity *e, EntitySystem *system)
{
   auto aabb = ComponentHelpers::getCollisionBox(e);

   aabb.offset(Float2(0.0f, 0.1f));
   auto gridCollisions = system->getManager<GridManager>()->collisionAt(aabb);

   for(auto block : gridCollisions)
   {
      auto blockBounds = ComponentHelpers::getCollisionBox(block);
      if(blockBounds.contains(aabb))
      {
         return true;
      }
   }

   return false;
}

//optionally returns the y-overlap if on head
boost::optional<float> characterOnHead(Entity *e, Entity *other, float yoffset = 0.0f)
{
   boost::optional<float> out;

   if(e != other)
   if(other->get<CharacterComponent>())
   {      
      if(auto evc = e->get<VelocityComponent>())
      if(auto cc = e->get<TCharacterComponent>())
      {
         auto ebox = ComponentHelpers::getCollisionBox(e);

         ebox.offset(Float2(0.0f, yoffset));

         auto obox = ComponentHelpers::getCollisionBox(other);
         auto yoverlap = ebox.bottom - obox.top;
         auto xoverlap = std::min(ebox.right - obox.left, obox.right - ebox.left);
         if(yoverlap > -0.0f && 
            yoverlap < (obox.height() / 2.0f) && xoverlap > ((obox.width() / 3.0f)))
         {
            float ov = 0.0f;
            //if(auto ovc = other->get<VelocityComponent>())
               //ov = ovc->velocity.y;

            out = yoverlap + ov;
         }
      }
   }

   return out;

}
bool characterFalling(Entity *e)
{
   if(auto vc = e->get<VelocityComponent>())
      return vc->velocity.y > 0.0f;

   return false;
}
Entity *characterGetCarrier(Entity *e)
{
   if(auto cc = e->get<CarriedComponent>())
      return cc->carrier;

   return nullptr;
}

void characterMakeCarried(Entity *e, Entity *carrier)
{
   e->add(CarriedComponent(carrier));
}
void characterRemoveCarried(Entity *e)
{
   e->remove<CarriedComponent>();
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
            m_players[cc->playerNumber]->add(TRemainingTimeComponent());
            m_players[cc->playerNumber]->add(TCharacterComponent());
            m_players[cc->playerNumber]->get<TCharacterComponent>()->sm->set(buildAirState(m_players[cc->playerNumber]));
         } 
   }
   void onDelete(Entity *e)
   {
      if(auto cc = e->get<CharacterComponent>())
         if(cc->playerNumber < GameData::PlayerCount && m_players[cc->playerNumber] == cc->parent)
         {
            m_players[cc->playerNumber]->remove<TRemainingTimeComponent>();
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
    

   CharacterState *buildCarriedState(Entity *e)
   {
      class CarriedState : public CharacterState
      {
         CharacterManagerImpl *m_manager;
      public:
         CarriedState(CharacterManagerImpl *manager, EntitySystem *system, Entity *e)
            :m_manager(manager), CharacterState(system, e){}

         bool shouldFall()
         {
            if(auto carrier = characterGetCarrier(e))
            {
               if(auto overlap = characterOnHead(e, carrier, 0.5f))
               {
                  if(*overlap > 1.0f)
                  {
                     characterOffsetPosition(e, m_system, 0.0f, -*overlap);                     
                  }
                  return false;
               }
            }
            return true;
         }
         virtual void onLeave()
         {
            characterRemoveCarried(e);
         }
         virtual void update()
         {   
            characterUpdateCarriedVelocity(e, m_system);
            characterUpdateCarriedMovement(e);

            auto n = characterCheckGridCollision(e, m_system);

            if((n && n->y > 0.0f) || shouldFall())
            {
               Logs::e("Character") << "Fell off!";
               characterSetState(e, m_manager->buildAirState(e));
            }
            else
               characterUpdateGroundSprite(e);
         }

         virtual void jump()
         {
            if(characterJump(e))
               characterSetState(e, m_manager->buildAirState(e));
         }

         virtual void moveLeft(){characterMoveLeft(e);}
         virtual void moveRight(){characterMoveRight(e);}
         virtual void stop(){characterStop(e);}

      };

      return new CarriedState(this, m_system, e);
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
            if(!characterOnGround(e, m_system))
            {
               characterSetState(e, m_manager->buildAirState(e));
               return;
            }          
            
            characterUpdateGroundMovement(e);
            characterCheckGridCollision(e, m_system);
            characterUpdateGroundSprite(e);
         }

         virtual void jump() {characterJump(e);}
         virtual void moveLeft(){characterMoveLeft(e);}
         virtual void moveRight(){characterMoveRight(e);}
         virtual void stop(){characterStop(e);}

      };

      return new GroundState(this, m_system, e);
   }

   CharacterState *buildAirState(Entity *e)
   {
      class AirState : public CharacterState
      {
         CharacterManagerImpl *m_manager;
      public:
         AirState(CharacterManagerImpl *manager, EntitySystem *system, Entity *e)
            :m_manager(manager), CharacterState(system, e){}

         virtual void resolveCollision(std::vector<Entity*> eList)
         {
            for(auto other : eList)
            { 
               if(auto overlap = characterOnHead(e, other))
               {
                  if(characterFalling(e) && characterOffsetPosition(e, m_system, 0, -*overlap))
                  {
                     characterMakeCarried(e, other);
                     characterEndJump(e);
                     characterSetState(e, m_manager->buildCarriedState(e));
                     Logs::w("Collision") << "Jumped on a head!";
                     return;
                  }
               }
            }
         }

         virtual void update()
         {
            characterUpdateAirMovement(e, app.dt());

            if(auto normal = characterCheckGridCollision(e, m_system))
            {
               if(normal->y < -0.7f)
               {
                  //hit ground
                  characterSetState(e, m_manager->buildGroundState(e));
                  return;
               }
               else if(normal->y > 0.0f)
               {
                  //hit ceiling
                  characterEndJump(e);
               }
            }

            characterUpdateAirSprite(e);            
         }

         virtual void updateGravity() {characterUpdateGravity(e, app.dt());}
         virtual void endJump(){characterEndJump(e);}

         virtual void moveLeft(){characterMoveLeft(e);}
         virtual void moveRight(){characterMoveRight(e);}
         virtual void stop(){characterStop(e);}
      };

      return new AirState(this, m_system, e);
   }

   void updateGravity(std::vector<Entity*>& out)
   {
      for(auto e : out)
      {
         if(auto cc = e->get<TCharacterComponent>())
         if(auto &sm = *cc->sm)
         {
            sm->updateGravity();
         }
      }
   }

   void resolveCollisions(std::vector<Entity*>& out)
   {
      auto cpm = m_system->getManager<CollisionPartitionManager>();

      for(auto e : out)
      {
         if(auto cc = e->get<TCharacterComponent>())
         if(auto &sm = *cc->sm)
         {
            sm->resolveCollision(cpm->getEntities(ComponentHelpers::getCollisionBox(e)));
         }
      }
   }

   void performCharacterUpdates(std::vector<Entity*>& out)
   {
      for(auto e : out)
      {
         if(auto cc = e->get<TCharacterComponent>())
         if(auto &sm = *cc->sm)
         {
            sm->update();
         }
      }
   }
   

   void solveRecursive(Entity* entity, std::vector<Entity*>& out)
   {
      if (auto children = entity->get<TChildren>())
      {
         for (auto child : children->children)
         {
            out.push_back(child);
            solveRecursive(child, out);
         }
         entity->remove<TChildren>();
      }
   }
   std::vector<Entity*> carrySortedEntities()
   {
      std::vector<Entity*> out;

      for (auto comp : m_system->getComponentVector<TCharacterComponent>())
      {
         if(auto cc = comp.parent->get<CarriedComponent>())
         {
            if(!cc->carrier->get<TChildren>())
               cc->carrier->add(TChildren());

            cc->carrier->get<TChildren>()->children.push_back(comp.parent);
         }
         else
         {
            out.push_back(comp.parent);
         }
      }

      size_t count = out.size();
      for (size_t i = 0; i < count; ++i)
         solveRecursive(out[i], out);

      return out;
   }

   void resetRemainingTimes()
   {
      for (auto comp : m_system->getComponentVector<TCharacterComponent>())
      {
         if(auto rtc = comp.parent->get<TRemainingTimeComponent>())
            rtc->remainingTime = 1.0f;
      }
   }

   void update()   
   {
      resetRemainingTimes();
     
      auto eList = carrySortedEntities();

      
      
      updateGravity(eList);
      resolveCollisions(eList);
      performCharacterUpdates(eList);
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
   void stop(int player){characterStateFunction(player, [=](CharSM& sm){sm->stop();});}   
   void jump(int player){characterStateFunction(player, [=](CharSM& sm){sm->jump();});}
   void endJump(int player){characterStateFunction(player, [=](CharSM& sm){sm->endJump();});}
};

std::unique_ptr<CharacterManager> buildCharacterManager()
{
   return std::unique_ptr<CharacterManager>(new CharacterManagerImpl());
}