#pragma once

#include "engine\Entity.h"
#include "engine\Component.h"

class CharacterManager : public IManager
{
public:
   virtual void update()=0;

   virtual void moveLeft(int player)=0;
   virtual void moveRight(int player)=0;
   virtual void moveUp(int player)=0;
   virtual void moveDown(int player)=0;

   virtual void stopLeft(int player)=0;
   virtual void stopRight(int player)=0;
   virtual void stopUp(int player)=0;
   virtual void stopDown(int player)=0;

};



std::unique_ptr<CharacterManager> buildCharacterManager();