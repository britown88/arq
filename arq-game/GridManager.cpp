#include "GridManager.h"
#include "engine\Component.h"
#include "engine\CoreComponents.h"
#include "ArqComponents.h"
#include "ActorManager.h"

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

   Rectf getGridRect()
   {
      return Rectf(  origin.x, 
                     origin.y, 
                     origin.x + (gridDims.x * cellSize.x), 
                     origin.y + (gridDims.y * cellSize.y));
      
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

   void updateGridCollisions()
   {
      for (auto comp : m_system->getComponentVector<ActorComponent>())         
      {
         Float2 v;

         if(auto vc = comp.parent->get<VelocityComponent>())
            v = vc->velocity;

         bool moveLeft = v.x < 0,
         moveRight = v.x > 0,
         moveUp = v.y < 0,
         moveDown = v.y > 0;
       
         bool moveVert = moveUp || moveDown;
         bool moveHorz = moveLeft || moveRight;

         if(!moveHorz && !moveVert)
            continue;

          //future aabbs and for only vertical or only horizontal movement
         Rectf bounds = ComponentHelpers::getCollisionBox(comp.parent);
         Rectf fAABB, fHorzAABB, fVertAABB;
         fAABB = fHorzAABB = fVertAABB = bounds;

         fAABB.offset(v);
 
         auto collidingBlocks = collisionAt(fAABB);
         if(!collidingBlocks.empty()) 
         {
            if(moveHorz)
               fHorzAABB.offset(v.x, 0.0f);

            if(moveVert)
               fVertAABB.offset(0.0f, v.y);
 
            boost::optional<float> edgeLeft, edgeRight, edgeTop, edgeBottom;
            boost::optional<Float2> topLeft, topRight, bottomLeft, bottomRight;

            bool hitCorner = false;
   
            for(auto block : collidingBlocks) 
            {
               Rectf bbounds = ComponentHelpers::getCollisionBox(block);
       
               //skip if bordering an existing edge
               if(   edgeLeft && bbounds.left == *edgeLeft ||
                     edgeRight && bbounds.right == *edgeRight ||
                     edgeTop && bbounds.top == *edgeTop ||
                     edgeBottom && bbounds.bottom == *edgeBottom)
                  continue;
       
               //determine which of the mvoements caused the collision

               bool vertCollids = bbounds.contains(fVertAABB);
               bool horzCollides = bbounds.contains(fHorzAABB);

               bool causedByHorz = !moveVert || !vertCollids;
               bool causedByVert = !moveHorz || !horzCollides;

               if(causedByHorz && causedByVert)
               {
                  hitCorner = true;
                  //hit on a corner, save the corner info to defer until later
                  if(moveUp && moveLeft) bottomRight = Float2(bbounds.right, bbounds.bottom);
                  else if(moveUp && moveRight) bottomLeft = Float2(bbounds.left, bbounds.bottom);
                  else if(moveDown && moveLeft) topRight = Float2(bbounds.right, bbounds.top);
                  else if(moveDown && moveRight) topLeft = Float2(bbounds.left, bbounds.top);
               }
               else
               {
                  //set colliding edges accordingly
                  if(causedByHorz) 
                  {
                     if(moveLeft && (!edgeRight || bbounds.right > *edgeRight)) edgeRight = bbounds.right;
                     if(moveRight && (!edgeLeft || bbounds.left < *edgeLeft)) edgeLeft = bbounds.left;
                  }
       
                  if(causedByVert) 
                  {
                     if(moveUp && (!edgeBottom || bbounds.bottom > *edgeBottom)) edgeBottom = bbounds.bottom;
                     if(moveDown && (!edgeTop || bbounds.top < *edgeTop)) edgeTop = bbounds.top;
                  }
               }
            }

            //resolvecorners
            if(hitCorner)
            {
               //the goal here is to only use a corner to define edges if both of those edges never got set by other blocks
               if(topLeft && !edgeTop && !edgeLeft)
               {
                  if(fAABB.right - topLeft->x > fAABB.bottom - topLeft->y)
                     edgeTop = topLeft->y; 
                  else
                     edgeLeft = topLeft->x;
               }
               else if(topRight && !edgeTop && !edgeRight)
               {
                  if(topRight->x - fAABB.left > fAABB.bottom - topRight->y)
                     edgeTop = topRight->y; 
                  else
                     edgeRight = topRight->x;
               }
               else if(bottomLeft && !edgeBottom && !edgeLeft)
               {
                  if(fAABB.right - bottomLeft->x > bottomLeft->y - fAABB.top)
                     edgeBottom = bottomLeft->y; 
                  else
                     edgeLeft = bottomLeft->x;
               }
               else if(bottomRight && !edgeBottom && !edgeRight)
               {
                  if(bottomRight->x - fAABB.left > bottomRight->y - fAABB.top)
                     edgeBottom = bottomRight->y; 
                  else
                     edgeRight = bottomRight->x;
               }
            }
   
            //determine how to offset the actor depending on which edges exist
            Float2 offset;
   
            if(edgeLeft) 
            {
               offset.x += *edgeLeft - bounds.right;
               v.x = 0.0f;
            }
   
            if(edgeRight) 
            {
               offset.x -= bounds.left - *edgeRight;
               v.x = 0.0f;
            }
   
            if(edgeTop) 
            {
               offset.y += *edgeTop - bounds.bottom;
               v.y = 0.0f;
            }
   
            if(edgeBottom) 
            {
               offset.y -= bounds.top - *edgeBottom;
               v.y = 0.0f;
            }

            //offset.x += offset.x > 0.0f ? -GameMath::Epsilon : (offset.x < 0.0f ? GameMath::Epsilon : 0.0f);
            //offset.y += offset.y > 0.0f ? -GameMath::Epsilon : (offset.y < 0.0f ? GameMath::Epsilon : 0.0f);
   
            //update position and velocity
            if(auto pc = comp.parent->lock<PositionComponent>())
               pc->pos += offset;
       
            if(auto vc = comp.parent->lock<VelocityComponent>())
               vc->velocity = v;
         }
      }
   }
};

std::unique_ptr<GridManager> buildGridManager()
{
   return std::unique_ptr<GridManager>(new GridManagerImpl());
}

