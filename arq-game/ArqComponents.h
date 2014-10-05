#pragma once

#include "engine\Component.h"
#include "engine\Vector.h"
#include "engine\StringTable.h"

struct VelocityComponent : public Component
{
   Float2 velocity;
   Float2 maxVelocity;
   VelocityComponent(){}
   VelocityComponent(Float2 maxVelocity):maxVelocity(maxVelocity){}
};

struct TargetComponent : public Component
{
   Entity *target;
   TargetComponent(Entity *target):target(target){}
   TargetComponent():target(nullptr){}
};

struct AttackComponent : public Component
{

};

struct PlayerControlledComponent : public Component{};

struct ActorComponent : public Component
{
   InternString   upIdleSprite, 
                  downIdleSprite, 
                  leftIdleSprite, 
                  rightIdleSprite,
                  upRunSprite, 
                  downRunSprite, 
                  leftRunSprite,
                  rightRunSprite;

   InternString   mainHandAction,
                  offHandAction;

   bool lockFacing, lockMovement;

   ActorComponent():
                  upIdleSprite(nullptr),
                  downIdleSprite(nullptr),
                  leftIdleSprite(nullptr),
                  rightIdleSprite(nullptr),
                  upRunSprite(nullptr),
                  downRunSprite(nullptr),
                  leftRunSprite(nullptr),
                  rightRunSprite(nullptr),
                  mainHandAction(nullptr),
                  offHandAction(nullptr),
                  lockFacing(false), lockMovement(false){}
};



