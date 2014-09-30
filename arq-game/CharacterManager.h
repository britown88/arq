#pragma once

#include "engine\Entity.h"
#include "engine\Component.h"

struct TRemainingTimeComponent : public Component
{
   mutable float remainingTime;
   mutable Float2 deltaPosition;
   TRemainingTimeComponent():remainingTime(0.0f), deltaPosition(Float2()){}
};

class CharacterManager : public IManager
{
public:
   virtual void update()=0;


   virtual void moveLeft(int player)=0;
   virtual void moveRight(int player)=0;
   virtual void stop(int player)=0;
   virtual void jump(int player)=0;
   virtual void endJump(int player)=0;
};



std::unique_ptr<CharacterManager> buildCharacterManager();