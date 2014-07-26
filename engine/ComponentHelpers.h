#pragma once

#include "Matrix.h"
class Entity;

namespace ComponentHelpers
{
   Rectf getEntityRect(Entity *e);

   Entity *getBaseParent(Entity *e);
   float getBottomY(Entity *e);
   float getSkeletalZ(Entity *e);
   int getLayer(Entity *e);

   void buildEntityTextureTransform(Entity *e, Matrix &m);


};

 