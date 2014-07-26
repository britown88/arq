#include "MatrixManager.h"

#include "CoreComponents.h"
#include "Skeletal.h"

struct TMatrixComponent : public Component
{
   mutable Matrix scaled, unscaled;
   mutable bool dirty;
   TMatrixComponent():dirty(true){}
};

REGISTER_COMPONENT(TMatrixComponent)

class MatrixManagerImpl : public Manager<MatrixManagerImpl, MatrixManager>
{
   void updateSkeletalLimb(Entity *e, const TMatrixComponent &mc)
   {
      if(auto skeletalLimb = e->get<SkeletalLimbComponent>())
      {
         if(auto parent = skeletalLimb->skeletalParent)
         {
            if(auto parentSkeleton = parent->get<SkeletalLimbComponent>())
            if(auto parentMatrix = parent->get<TMatrixComponent>())
            //if(auto parentgbc = parent->get<GraphicalBoundsComponent>())
            {
               if(parentMatrix->dirty)
                  clean(skeletalLimb->skeletalParent);

               auto &nodePos = parentSkeleton->getNodePosition(skeletalLimb->attachedNode);
               Matrix t = parentMatrix->unscaled;

               //MatrixTransforms::translate(t, -parentgbc->center.x, -parentgbc->center.y);
               MatrixTransforms::translate(t, nodePos.x, nodePos.y);

               mc.unscaled = mc.unscaled * t;
            }
         }
      }
   }

   void clean(Entity *e)
   {
      if(auto mc = e->get<TMatrixComponent>())
      {
         Float2 pos;
         Float2 size(1, 1);

         auto pc = e->get<PositionComponent>();
         auto gbc = e->get<GraphicalBoundsComponent>();
         auto rc = e->get<RotationComponent>();

         if(pc)
         {
            pos.x = pc->pos.x;
            pos.y = pc->pos.y;
         }

         if(gbc)
         {
            size.x = gbc->size.x;
            size.y = gbc->size.y;

            pos.x -= gbc->center.x;
	         pos.y -= gbc->center.y;

         }

         MatrixTransforms::identity(mc->unscaled);
         updateSkeletalLimb(e, *mc);
         MatrixTransforms::translate(mc->unscaled, pos.x, pos.y);
         if(rc)
         {
            if(rc->angle != 0.0f)
            {
               MatrixTransforms::translate(mc->unscaled, rc->point.x, rc->point.y);
               MatrixTransforms::rotate(mc->unscaled, rc->angle);
               MatrixTransforms::translate(mc->unscaled, -rc->point.x, -rc->point.y);
            }
         }
         
         mc->scaled = mc->unscaled;   
         MatrixTransforms::scale(mc->scaled, size.x, size.y);

         mc->dirty = false;
      }
   }

   void makeDirty(Entity *e)
   {
      if(auto mc = e->get<TMatrixComponent>())
         mc->dirty = true;

      //dirty the children
      if(auto skel = e->get<SkeletalLimbComponent>())
      {
         for(auto &&child : skel->nodes)
            makeDirty(child.second.entity);
      }
   }

public:
   MatrixManagerImpl(){}
   ~MatrixManagerImpl(){}

   static void registerComponentCallbacks(Manager<MatrixManagerImpl, MatrixManager> &m)
   {
      m.add<PositionComponent>();
      m.add<GraphicalBoundsComponent>();
      m.add<RotationComponent>();
   }

   void onChanged(Entity *e, const PositionComponent &oldData, const PositionComponent &newData, int key){makeDirty(e);}
   void onAdded(Entity *e, const PositionComponent &comp, int key){makeDirty(e);}
   void onRemoved(Entity *e, const PositionComponent &comp, int key){makeDirty(e);}

   void onChanged(Entity *e, const GraphicalBoundsComponent &oldData, const GraphicalBoundsComponent &newData, int key){makeDirty(e);}
   void onAdded(Entity *e, const GraphicalBoundsComponent &comp, int key){makeDirty(e);}
   void onRemoved(Entity *e, const GraphicalBoundsComponent &comp, int key){makeDirty(e);}

   void onChanged(Entity *e, const RotationComponent &oldData, const RotationComponent &newData, int key){makeDirty(e);}
   void onAdded(Entity *e, const RotationComponent &comp, int key){makeDirty(e);}
   void onRemoved(Entity *e, const RotationComponent &comp, int key){makeDirty(e);}

   void onNew(Entity *e)
   {
      e->add(TMatrixComponent());
   }

   void onDelete(Entity *e)
   {
      e->remove<TMatrixComponent>();
   }

   const Matrix *getMatrix(Entity *e)
   {
      if(auto mc = e->get<TMatrixComponent>())
      {
         if(mc->dirty)
            clean(e);

         return &mc->scaled;
      }

      return nullptr;
   }
};


std::unique_ptr<MatrixManager> buildMatrixManager()
{
   return std::unique_ptr<MatrixManager>(new MatrixManagerImpl());
}
