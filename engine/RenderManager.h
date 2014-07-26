#pragma once

#include "Entity.h"
#include "Renderer.h"

class RenderManager : public IManager, public IRenderable
{
public:
};

std::unique_ptr<RenderManager> buildRenderManager(int partitionIndex);
