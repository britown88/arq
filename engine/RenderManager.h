#pragma once

#include "Entity.h"
#include "Renderer.h"

class RenderManager : public IManager
{
public:
   virtual void draw(Renderer &r, Rectf drawBounds)=0;
};


