#pragma once

#include "Matrix.h"
class Entity;

namespace ComponentHelpers
{
   Rectf getEntityRect(Entity *e);

   int getLayer(Entity *e);

   void buildEntityTextureTransform(Entity *e, Matrix &m);

   Rectf getCollisionBox(Entity *e);


};

 