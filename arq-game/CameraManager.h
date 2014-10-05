#pragma once

#include "engine\Entity.h"
#include "engine\Component.h"
#include  "engine\CoreUI.h"

struct CameraComponent : public Component{};

class CameraManager : public IManager
{
public:
   virtual void update()=0;

   virtual Entity *getCameraEntity()=0;
};

std::unique_ptr<CameraManager> buildCameraManager(CoreUI::WorldUIElement *world);