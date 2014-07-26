#include "PartitionManager.h"
#include "PagedAllocator.h"
#include "ComponentHelpers.h"
#include "CoreComponents.h"
#include "Skeletal.h"

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

int getNewPartitionSelectionIndex()
{
   static int index = 0;
   return index++;
}

struct TPartitionComponent : public Component
{
   mutable std::vector<PartitionCell *> cells;
   mutable Rectf bounds;
   mutable int mailbox;
   TPartitionComponent():mailbox(0){}
};

REGISTER_COMPONENT(TPartitionComponent)
REGISTER_COMPONENT(PartitionSelectionComponent)

static const int PartitionPixelSize = 500;
static const float InversePartSize = 1.0f / PartitionPixelSize;

class PartitionManagerImpl : public Manager<PartitionManagerImpl, PartitionManager>
{
   std::unique_ptr<PartitionTable> m_table;

   Recti getPartitionIndices(const Rectf &r)
   {
      return Recti(
         std::floorf(r.left * InversePartSize), 
         std::floorf(r.top * InversePartSize),
         std::ceilf(r.right * InversePartSize),
         std::ceilf(r.bottom * InversePartSize));
   }

   void removeEntityFromCell(Entity *e, PartitionCell *cell)
   {
      cell->entities.erase(std::find(begin(cell->entities), end(cell->entities), e));
      if(cell->entities.empty())
         m_table->removeCell(cell);
   }

   void repositionEntity(Entity *e)
   {
      if(auto tpc = e->get<TPartitionComponent>())
      {
         tpc->bounds = ComponentHelpers::getEntityRect(e);

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

   void addPartitionSelectionComponent(Entity *e, int index, int mailSlot)
   {
      if(auto select = e->get<PartitionSelectionComponent>(index))
         select->mailbox = mailSlot;
      else
         e->add(PartitionSelectionComponent(mailSlot), index);

      if(auto skel = e->get<SkeletalLimbComponent>())
      {
         for(auto &&child : skel->nodes)
            addPartitionSelectionComponent(child.second.entity, index, mailSlot);
               
      }
   }

public:
   PartitionManagerImpl():m_table(new PartitionTable()){}

   static void registerComponentCallbacks(Manager<PartitionManagerImpl, PartitionManager> &m)
   {
      m.add<PositionComponent>();
      m.add<GraphicalBoundsComponent>();
      m.add<SkeletalLimbComponent>();
   }

   //need to watch for skeletal limbs being attached and detached to 
   //determine if they should have partition data
   void onChanged(Entity *e, const SkeletalLimbComponent &oldData, const SkeletalLimbComponent &newData, int key)
   {
      if(oldData.skeletalParent != newData.skeletalParent)
      {
         if(newData.skeletalParent)
            onDelete(e);
         else
            onNew(e);
      }
      
   }
   void onAdded(Entity *e, const SkeletalLimbComponent &comp, int key){}
   void onRemoved(Entity *e, const SkeletalLimbComponent &comp, int key){}

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

   void removeSelectionComponents(int selectionIndex, int mailSlot)
   {
      auto &list = m_system->getComponentVector<PartitionSelectionComponent>(selectionIndex);

      for(size_t i = 0; i < list.size();)
      {
         if(list[i].mailbox != mailSlot)
            list[i].parent->remove<PartitionSelectionComponent>(selectionIndex);
         else
            ++i;
      }
   }

   void getEntities(const Rectf &area, int selectionIndex)
   {
      static int mailSlot = 0;
      ++mailSlot;

      Recti indices = getPartitionIndices(area);

      for(int y = indices.top; y <= indices.bottom; ++y)
      for(int x = indices.left; x <= indices.right; ++x)
      {
         if(auto cell = m_table->tryGetCell(x, y))//see if theres a cell
         {
            for(auto e : cell->entities)//loop over the entities
            {
               auto tpc = e->get<TPartitionComponent>();//get the transient

               if(tpc->mailbox != mailSlot)//has it been updated in this loop
               {
                  if(area.contains(tpc->bounds))//does it actually intersect
                  {
                     tpc->mailbox = mailSlot;
                     //e->add(PartitionSelectionComponent(), selectionIndex);
                     addPartitionSelectionComponent(e, selectionIndex, mailSlot);
                     
                  }
               }
            }
         }
      }

      removeSelectionComponents(selectionIndex, mailSlot);
   }

   void onNew(Entity *e)
   {
      //dont add if it is a child
      if(auto skel = e->get<SkeletalLimbComponent>())
         if(skel->skeletalParent)
            return;

      if(e->get<PositionComponent>() && e->get<GraphicalBoundsComponent>())
      {
         TPartitionComponent pc;
         pc.bounds = ComponentHelpers::getEntityRect(e);
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
      if(auto pc = e->get<TPartitionComponent>())
      {
         for(auto cell : pc->cells)
            removeEntityFromCell(e, cell);

         e->remove<TPartitionComponent>();
      }
   }

};

std::unique_ptr<PartitionManager> buildPartitionManager()
{
   return std::unique_ptr<PartitionManager>(new PartitionManagerImpl());
}
