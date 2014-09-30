#pragma once

#include "engine\Entity.h"

class CollisionManager : public IManager
{
public:
   virtual void update()=0;
};

std::unique_ptr<CollisionManager> buildCollisionManager();