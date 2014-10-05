#pragma once

#include "engine\Entity.h"
#include "engine\Component.h"
#include "GameData.h"

class ActorManager : public IManager
{
public:
   virtual void update()=0;

   virtual void moveLeft(Entity *e)=0;
   virtual void moveRight(Entity *e)=0;
   virtual void moveUp(Entity *e)=0;
   virtual void moveDown(Entity *e)=0;

   virtual void stopLeft(Entity *e)=0;
   virtual void stopRight(Entity *e)=0;
   virtual void stopUp(Entity *e)=0;
   virtual void stopDown(Entity *e)=0;

   virtual void executeAction(Entity *e, ActionType type, Float2 target)=0;
   virtual void endAction(Entity *e, ActionType type, Float2 target)=0;

};

std::unique_ptr<ActorManager> buildActorManager();