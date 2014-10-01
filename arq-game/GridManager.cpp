#include "GridManager.h"
#include "engine\Component.h"
#include "engine\CoreComponents.h"
#include "ArqComponents.h"
#include "CharacterManager.h"

#include "engine\Logs.h"
#include "engine\Application.h"
#include "engine\IOCContainer.h"
#include "engine\GameMath.h"
#include "engine\ComponentHelpers.h"

REGISTER_COMPONENT(GridComponent);

struct TGridComponent : public Component
{
   mutable int mailbox;
   TGridComponent():mailbox(0){}
};

static int mailSlot = 0;

REGISTER_COMPONENT(TGridComponent);

class GridManagerImpl : public Manager<GridManagerImpl, GridManager>
{
   std::vector<Entity*> m_grid;
   Float2 origin;
   Float2 cellSize;
   Float2 invCellSize;
   Int2 gridDims;

   Entity *&entityAt(int x, int y)
   {
      return m_grid[y * gridDims.x + x];
   }

public:
   static void registerComponentCallbacks(Manager<GridManagerImpl, GridManager> &m)
   {
   }

   void onNew(Entity *e)
   {
      if(auto gc = e->get<GridComponent>())
      {
         if(auto pc = e->lock<PositionComponent>())
         {
            pc->pos.x = origin.x + cellSize.x * gc->cellRange.left;
            pc->pos.y = origin.y + cellSize.y * gc->cellRange.top;
         }

         if(auto gbc = e->lock<GraphicalBoundsComponent>())
         {
            gbc->size.x = cellSize.x * gc->cellRange.width();
            gbc->size.y = cellSize.y * gc->cellRange.height();
         }

         if(auto cbc = e->lock<CollisionBoxComponent>())
         {
            cbc->aabb.left = 0.0f;
            cbc->aabb.top = 0.0f;
            cbc->aabb.right = cellSize.x * gc->cellRange.width();
            cbc->aabb.bottom = cellSize.y * gc->cellRange.height();
         }

         for(int y = gc->cellRange.top; y < gc->cellRange.bottom; ++y)
         {
            for(int x = gc->cellRange.left; x < gc->cellRange.right; ++x)
            {
               if(y >= 0 && y < gridDims.y && x >= 0 && x < gridDims.x)
               {
                  entityAt(x, y) = e;
               }
            }
         }

         e->add(TGridComponent());
            
      }
   }

   void onDelete(Entity *e)
   {
      if(auto gc = e->get<GridComponent>())
      {
         for(int y = gc->cellRange.top; y < gc->cellRange.bottom; ++y)
         {
            for(int x = gc->cellRange.left; x < gc->cellRange.right; ++x)
            {
               if(y >= 0 && y < gridDims.y && x >= 0 && x < gridDims.x)
               {
                  entityAt(x, y) = nullptr;
               }
            }
         }

         e->remove<TGridComponent>();
            
      }
   }

   void createGrid(Float2 origin, Int2 size, Float2 cellSize)
   {
      this->gridDims = size;
      this->origin = origin;
      this->cellSize = cellSize;
      this->invCellSize = Float2(1.0f / cellSize.x, 1.0f / cellSize.y);
      m_grid.resize(size.x * size.y, nullptr);
   }

   Int2 gridPosition(Float2 worldPos)
   {
      return Int2((worldPos.x - origin.x) * invCellSize.x, (worldPos.y - origin.y) * invCellSize.y);
   }

   Entity *entityAt(Int2 gridPos)
   {
      if(gridPos.x >= 0 && gridPos.x < gridDims.x && 
         gridPos.y >= 0 && gridPos.y < gridDims.y)
         return entityAt(gridPos.x, gridPos.y);

      return nullptr;
   }

   std::vector<Entity*> collisionAt(Rectf bounds)
   {
      ++mailSlot;
      Recti gridPos(
         (bounds.left - origin.x) * invCellSize.x,
         (bounds.top - origin.y) * invCellSize.y,
         (bounds.right - origin.x) * invCellSize.x,
         (bounds.bottom - origin.y) * invCellSize.y);

      std::vector<Entity*> ret;

      for(int y = gridPos.top; y <= gridPos.bottom; ++y)
      {
         for(int x = gridPos.left; x <= gridPos.right; ++x)
         {
            if(y >= 0 && y < gridDims.y && x >= 0 && x < gridDims.x)
            {
               if(auto &e = entityAt(x, y))
               {
                  if(auto tgc = e->get<TGridComponent>())
                  {
                     if(tgc->mailbox != mailSlot)
                     {
                        ret.push_back(e);
                        tgc->mailbox = mailSlot;
                     }
                  }
               }
            }
         }
      }

      return ret;
   }
};

std::unique_ptr<GridManager> buildGridManager()
{
   return std::unique_ptr<GridManager>(new GridManagerImpl());
}

bool checkGridCollision(EntitySystem *system, Entity *e, Float2 &normalOut, float& remainingTime)
{
   float t = remainingTime * IOC.resolve<Application>()->dt();
   bool hasCollision = false;

   Float2 savedPos;

   if(auto pc = e->lock<PositionComponent>())
   if(auto vc = e->lock<VelocityComponent>())
   {
      savedPos = pc->pos;
      auto aabb = ComponentHelpers::getCollisionBox(e);
      Float2 scaledVelocity = vc->velocity * t;

      auto futureaabb = aabb;
      futureaabb.offset(scaledVelocity);
      auto gridCollisions = system->getManager<GridManager>()->collisionAt(futureaabb);

      if(!gridCollisions.empty())
      {
         float moveAmnt = 1.0f;
         Float2 normal, colNormal;
         for(auto block : gridCollisions)
         {
            auto blockBounds = ComponentHelpers::getCollisionBox(block);
            auto time = GameMath::offsetCollisionTime(aabb, scaledVelocity, blockBounds, colNormal) - GameMath::Epsilon;

            if(time < moveAmnt)
            {
               moveAmnt = time;
               normal = colNormal;
            }
         }

         //clipped into ground then clipped into wall
         //the problem here is that we aren't updating the position here based on the velocity, things actually move *faster*
         //when colliding because they are offset then run! D:
         if(moveAmnt < 1.0f)
         {
            if (moveAmnt < 0.0f) moveAmnt = 0.0f;

            remainingTime -= moveAmnt;

            pc->pos.x += scaledVelocity.x * moveAmnt;
            pc->pos.y += scaledVelocity.y * moveAmnt;
                     
            auto next = GameMath::perp2D(normal);
            float scale = GameMath::dot(next, vc->velocity);

            vc->velocity.x = next.x * scale;
            vc->velocity.y = next.y * scale;

            normalOut = normal;

            hasCollision = true;
         }
      }
   }

   if (hasCollision)
   {
      //damping
      remainingTime -= 0.1f;
      if (remainingTime > 0.0f)
      {
         checkGridCollision(system, e, normalOut, remainingTime);
      }

      return true;
   }

   return false;
}
