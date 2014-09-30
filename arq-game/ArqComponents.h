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

struct CharacterComponent : public Component
{
   int playerNumber;

   InternString runSprite, idleSprite, jumpUpSprite, jumpDownSprite;

   CharacterComponent():playerNumber(playerNumber), runSprite(nullptr),  
      idleSprite(nullptr),  jumpUpSprite(nullptr),  jumpDownSprite(nullptr){}
};

struct CarriedComponent : public Component
{
   Entity *carrier;
   CarriedComponent(Entity *carrier):carrier(carrier){}
};



