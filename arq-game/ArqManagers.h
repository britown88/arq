#pragma once

#include "engine\Entity.h"
#include "engine\RenderManager.h"
#include "engine\UIElement.h"
#include "engine\CoreUI.h"

class PhysicsManager : public IManager{public:virtual void update()=0;};
std::unique_ptr<PhysicsManager> buildPhysicsManager();

std::unique_ptr<RenderManager> buildRenderManager();

class InputManager : public IManager{public:};
std::unique_ptr<InputManager> buildInputManager(CoreUI::WorldUIElement *element);

class CollisionManager : public IManager{public:virtual void update()=0;};
std::unique_ptr<CollisionManager> buildCollisionManager();

class AttackManager : public IManager{public:virtual void update()=0;};
std::unique_ptr<AttackManager> buildAttackManager();

class TimedLifeManager : public IManager{public:virtual void update()=0;};
std::unique_ptr<TimedLifeManager> buildTimedLifeManager();
