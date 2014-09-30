#include "PartitionManager.h"
#include "GridManager.h"
#include "engine/PagedAllocator.h"
#include "engine/ComponentHelpers.h"
#include "engine/CoreComponents.h"

#include <vector>

#pragma region PartitionStuff
static const int CellCount = 262144;

class PartitionCell
{
public:
   PartitionCell *prev, *next;
   int x, y;
   std::vector<Entity *> entities;

   void *operator new(size_t count)
   {
      return PagedAllocator<PartitionCell>::instance().alloc();
   }

   void operator delete(void *ptr)
   {
      PagedAllocator<PartitionCell>::instance().free((PartitionCell*)ptr);
   }
};

class PartitionCellDummy
{
public:
   PartitionCell *prev, *next;
};

class PartitionTable
{
   size_t hash(int x, int y)
   {
      size_t out = (((size_t)x) << 5) + (((size_t)x) << 1) + 5381;
      return out + (size_t)y;
   }

   PartitionCellDummy m_cells[CellCount];
public:
   PartitionTable()
   {
      for(auto &cell : m_cells)
         cell.next = cell.prev = (PartitionCell*)&cell;
   }

   PartitionCell *tryGetCell(int x, int y)
   {
      auto index = hash(x, y) & (CellCount - 1);
      auto &node = m_cells[index];
      auto iter = node.next;

      while(iter != (PartitionCell *)&node)
      {
         if(iter->x == x && iter->y == y)
            return iter;
      }

      return nullptr;
   }

   PartitionCell *getCell(int x, int y)
   {
      auto index = hash(x, y) & (CellCount - 1);
      auto &node = m_cells[index];
      auto iter = node.next;

      while(iter != (PartitionCell *)&node)
      {
         if(iter->x == x && iter->y == y)
            return iter;
      }

      //we didnt find one so create a cell at that point
      auto *newCell = new PartitionCell();
      
      newCell->next = node.next;
      newCell->next->prev = newCell;
      newCell->prev = (PartitionCell*)&node;
      node.next = newCell;

      newCell->x = x;
      newCell->y = y;

      return newCell;
   }

   void removeCell(int x, int y)
   {
      if(auto cell = tryGetCell(x, y))
         removeCell(cell);
   }

   void removeCell(PartitionCell *cell)
   {
      cell->next->prev = cell->prev;
      cell->prev->next = cell->next;
      delete cell;
   }
};

#pragma endregion

template<typename StaticInterface>
struct TPartitionComponent : public Component
{
   mutable std::vector<PartitionCell *> cells;
   mutable Rectf bounds;
   mutable int mailbox;
   TPartitionComponent():mailbox(0){}
};

struct GraphicalStaticInterface
{
   typedef GraphicalTag TAG;
   template <typename T>
   static void registerCallbacks(T& registerFn)
   {
      registerFn.add<PositionComponent>();
      registerFn.add<GraphicalBoundsComponent>();
   }
   boost::optional<Rectf> getEntityRect(Entity* e)
   {
      if(e->get<PositionComponent>() && 
         e->get<GraphicalBoundsComponent>() &&
        !e->get<GridComponent>())
      {
         return ComponentHelpers::getEntityRect(e);
      }
      return boost::none;
   }


   void onChanged(Entity *e, const PositionComponent &oldData, const PositionComponent &newData, int key)
   {
      repositionEntity(e);
   }
   void onAdded(Entity *e, const PositionComponent &comp, int key){}
   void onRemoved(Entity *e, const PositionComponent &comp, int key){}

   void onChanged(Entity *e, const GraphicalBoundsComponent &oldData, const GraphicalBoundsComponent &newData, int key)
   {
      repositionEntity(e);
   }
   void onAdded(Entity *e, const GraphicalBoundsComponent &comp, int key){}
   void onRemoved(Entity *e, const GraphicalBoundsComponent &comp, int key){}


   virtual void repositionEntity(Entity* e)=0;
};
struct CollisionStaticInterface
{
   typedef CollisionTag TAG;
   template <typename T>
   static void registerCallbacks(T& registerFn)
   {
      registerFn.add<PositionComponent>();
      registerFn.add<CollisionBoxComponent>();
   }
   boost::optional<Rectf> getEntityRect(Entity* e)
   {
      if(e->get<PositionComponent>() && 
         e->get<CollisionBoxComponent>() &&
        !e->get<GridComponent>())
      {
         return ComponentHelpers::getCollisionBox(e);
      }
      return boost::none;
   }


   void onChanged(Entity *e, const PositionComponent &oldData, const PositionComponent &newData, int key)
   {
      repositionEntity(e);
   }
   void onAdded(Entity *e, const PositionComponent &comp, int key){}
   void onRemoved(Entity *e, const PositionComponent &comp, int key){}

   void onChanged(Entity *e, const CollisionBoxComponent &oldData, const CollisionBoxComponent &newData, int key)
   {
      repositionEntity(e);
   }
   void onAdded(Entity *e, const CollisionBoxComponent &comp, int key){}
   void onRemoved(Entity *e, const CollisionBoxComponent &comp, int key){}

   virtual void repositionEntity(Entity* e)=0;
};

typedef TPartitionComponent<GraphicalStaticInterface> TGraphicalPartitionComponent;
typedef TPartitionComponent<CollisionStaticInterface> TCollisionPartitionComponent;

//REGISTER_COMPONENT(TGraphicalPartitionComponent)
REGISTER_COMPONENT(TGraphicalPartitionComponent)
REGISTER_COMPONENT(TCollisionPartitionComponent)

static const int PartitionPixelSize = 500;
static const float InversePartSize = 1.0f / PartitionPixelSize;


template <typename StaticInterface> 
class PartitionManagerImpl : 
   public Manager<PartitionManagerImpl<StaticInterface>, 
      PartitionManager<typename StaticInterface::TAG>>, 
   public StaticInterface
{
   typedef TPartitionComponent<StaticInterface> TransientComp;
   std::unique_ptr<PartitionTable> m_table;

   Recti getPartitionIndices(const Rectf &r)
   {
      return Recti(
         (int)std::floorf(r.left * InversePartSize), 
         (int)std::floorf(r.top * InversePartSize),
         (int)std::ceilf(r.right * InversePartSize),
         (int)std::ceilf(r.bottom * InversePartSize));
   }

   void removeEntityFromCell(Entity *e, PartitionCell *cell)
   {
      cell->entities.erase(std::find(begin(cell->entities), end(cell->entities), e));
      if(cell->entities.empty())
         m_table->removeCell(cell);
   }

   void repositionEntity(Entity *e)
   {
      if(auto tpc = e->get<TransientComp>())
      {
         tpc->bounds = *getEntityRect(e);

         Recti indices = getPartitionIndices(tpc->bounds);

         for(auto cell : tpc->cells)
            removeEntityFromCell(e, cell);

         tpc->cells.clear();

         for(int y = indices.top; y <= indices.bottom; ++y)
         for(int x = indices.left; x <= indices.right; ++x)
         {
            auto cell = m_table->getCell(x, y);
            cell->entities.push_back(e);
            tpc->cells.push_back(cell);
         }      
      }
   }


public:
   PartitionManagerImpl():m_table(new PartitionTable()){}

   static void registerComponentCallbacks(Manager<PartitionManagerImpl, PartitionManager<typename StaticInterface::TAG>> &m)
   {
      StaticInterface::registerCallbacks(m);
   }

   std::vector<Entity*> getEntities(const Rectf &area)
   {
      static int mailSlot = 0;
      ++mailSlot;

      std::vector<Entity*> out;

      Recti indices = getPartitionIndices(area);

      for(int y = indices.top; y <= indices.bottom; ++y)
      for(int x = indices.left; x <= indices.right; ++x)
      {
         if(auto cell = m_table->tryGetCell(x, y))//see if theres a cell
         {
            for(auto e : cell->entities)//loop over the entities
            {
               auto tpc = e->get<TransientComp>();//get the transient

               if(tpc->mailbox != mailSlot)//has it been updated in this loop
               {
                  if(area.contains(tpc->bounds))//does it actually intersect
                  {
                     tpc->mailbox = mailSlot;
                     out.push_back(e);
                     
                  }
               }
            }
         }
      }

      return out;
   }

   void onNew(Entity *e)
   {
      if(auto bounds = getEntityRect(e))
      {
         TransientComp pc;
         pc.bounds = *bounds;
         Recti indices = getPartitionIndices(pc.bounds);

         for(int y = indices.top; y <= indices.bottom; ++y)
         for(int x = indices.left; x <= indices.right; ++x)
         {
            auto *cell = m_table->getCell(x, y);
            cell->entities.push_back(e);
            pc.cells.push_back(cell);

         }

         e->add(pc);
      }
   }

   void onDelete(Entity *e)
   {
      if(auto pc = e->get<TransientComp>())
      {
         for(auto cell : pc->cells)
            removeEntityFromCell(e, cell);

         e->remove<TransientComp>();
      }
   }

};

std::unique_ptr<GraphicalPartitionManager> buildGraphicalPartitionManager()
{
   return std::unique_ptr<GraphicalPartitionManager>(new PartitionManagerImpl<GraphicalStaticInterface>());
}

std::unique_ptr<CollisionPartitionManager> buildCollisionPartitionManager()
{
   return std::unique_ptr<CollisionPartitionManager>(new PartitionManagerImpl<CollisionStaticInterface>());
}

