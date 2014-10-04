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

struct ActorComponent : public Component
{
   int playerNumber;

   InternString   upIdleSprite, 
                  downIdleSprite, 
                  leftIdleSprite, 
                  rightIdleSprite,
                  upRunSprite, 
                  downRunSprite, 
                  leftRunSprite,
                  rightRunSprite;

   ActorComponent():playerNumber(playerNumber), 
                     upIdleSprite(nullptr),  
                     downIdleSprite(nullptr),  
                     leftIdleSprite(nullptr),  
                     rightIdleSprite(nullptr),  
                     upRunSprite(nullptr),
                     downRunSprite(nullptr),
                     leftRunSprite(nullptr),
                     rightRunSprite(nullptr){}
};



