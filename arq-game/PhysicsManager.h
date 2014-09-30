#pragma once

#include "engine\Entity.h"

class PhysicsManager : public IManager
{
public:
   virtual void update()=0;
};

std::unique_ptr<PhysicsManager> buildPhysicsManager();