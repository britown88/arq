#pragma once

#include "engine\Entity.h"
#include "engine\Vector.h"
#include "engine\Component.h"
#include "engine\Rect.h"

struct GridComponent : public Component
{
   Recti cellRange;
   GridComponent(){}
   GridComponent(Recti cellRange):cellRange(cellRange){}
};

class GridManager : public IManager
{
public:
   virtual void createGrid(Float2 origin, Int2 size, Float2 cellSize)=0;
   virtual std::vector<Entity*> collisionAt(Rectf bounds)=0;

   virtual Entity *entityAt(Int2 pos)=0;
   virtual Int2 gridPosition(Float2 worldPos)=0;

   virtual void updateGridCollisions()=0;

};


std::unique_ptr<GridManager> buildGridManager();

bool checkGridCollision(EntitySystem *system, Entity *e, Float2 &normalOut, float& remainingTime);