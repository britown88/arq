#include "RenderManager.h"

#include "CoreComponents.h"
#include "IOCContainer.h"
#include "TextureManager.h"
#include "Renderer.h"
#include "ComponentHelpers.h"
#include "Texture.h"
#include "PartitionManager.h"
#include "MatrixManager.h"
#include "SeanSort.h"

struct TRenderComponent : public Component
{
   mutable Texture *texture;
   mutable Matrix textureTransform;
   TRenderComponent():texture(nullptr){}
};

REGISTER_COMPONENT(TRenderComponent)

struct TVisibleComponent : public Component
{
};

REGISTER_COMPONENT(TVisibleComponent)

struct DrawnEntity
{
   DrawnEntity(Entity *e, const Matrix *transform):e(e), transform(transform){}
   Entity *e;
   const Matrix *transform;
};

class RenderManagerImpl : public Manager<RenderManagerImpl, RenderManager>
{
   int m_partitionIndex;

   void renderEntity(Renderer &r, const DrawnEntity &de)
   {
      Texture *tex = nullptr;
      auto tc = de.e->get<TRenderComponent>();
      if(tc)
         tex = tc->texture;

      auto mc = de.e->get<MeshComponent>();

      if(tex)
         r.drawTexture(mc->vbo, mc->ibo, tex, *de.transform, tc->textureTransform, mc->c);
      else
         r.drawTriangles(mc->vbo, mc->ibo, *de.transform, mc->c);
   }

public:
   RenderManagerImpl(int partitionIndex):m_partitionIndex(partitionIndex){}

   static void registerComponentCallbacks(Manager<RenderManagerImpl, RenderManager> &m)
   {
      m.add<TextureComponent>();
      m.add<PartitionSelectionComponent>();     
   }

   void onChanged(Entity *e, const TextureComponent &oldData, const TextureComponent &newData, int key)
   {
      if(auto t = e->get<TRenderComponent>())
      {
         TextureRequest request(newData.filePath);
         request.filterType = newData.filterType;
         request.repeatType = newData.repeatType;
         t->texture = IOC.resolve<TextureManager>()->getTexture(request);

         ComponentHelpers::buildEntityTextureTransform(e, t->textureTransform);
      }
   }
   void onAdded(Entity *e, const TextureComponent &comp, int key){}
   void onRemoved(Entity *e, const TextureComponent &comp, int key){}

   void onChanged(Entity *e, const PartitionSelectionComponent &oldData, const PartitionSelectionComponent &newData, int key){}
   void onAdded(Entity *e, const PartitionSelectionComponent &comp, int key)
   {
      if(key == m_partitionIndex && e->get<TRenderComponent>())
         e->add<TVisibleComponent>(TVisibleComponent());
   }
   void onRemoved(Entity *e, const PartitionSelectionComponent &comp, int key)
   {
      if(key == m_partitionIndex && e->get<TVisibleComponent>())
         e->remove<TVisibleComponent>();
   }

   void onNew(Entity *e)
   {
      if(e->get<MeshComponent>())
      {
         TRenderComponent trans;
         if(auto tex = e->get<TextureComponent>())
         {
            TextureRequest request(tex->filePath);
            request.filterType = tex->filterType;
            request.repeatType = tex->repeatType;
            trans.texture = IOC.resolve<TextureManager>()->getTexture(request);
            ComponentHelpers::buildEntityTextureTransform(e, trans.textureTransform);
         } 

         e->add(trans);
      }
      
   }

   void onDelete(Entity *e)
   {
      e->remove<TRenderComponent>();
   }

   void draw(Renderer &r)
   {
      auto mm = m_system->getManager<MatrixManager>();
      if(!mm) return;

      std::vector<DrawnEntity> eList;
      for(auto &comp : m_system->getComponentVector<TVisibleComponent>())
      {
         DrawnEntity de(comp.parent, mm->getMatrix(comp.parent));
         if(!de.transform)
            continue;

         eList.push_back(de);
      }

      eList = seanSort(std::move(eList), [&](const DrawnEntity &e1, const DrawnEntity &e2)->bool
      {
         auto parent1 = ComponentHelpers::getBaseParent(e1.e);
         auto parent2 = ComponentHelpers::getBaseParent(e2.e);

         //skeletal children
         if(parent1 == parent2)
         {
            auto z1 = ComponentHelpers::getSkeletalZ(e1.e);
            auto z2 = ComponentHelpers::getSkeletalZ(e2.e);

            if(z1 < z2)
               return true;
            if(z1 > z2)
               return false;

            return e1.e > e2.e;
         }

         //layering
         auto l1 = ComponentHelpers::getLayer(parent1);
         auto l2 = ComponentHelpers::getLayer(parent2);

         if(l1 < l2)
            return true;
         if(l1 > l2)
            return false; 

         //bottom Y
         auto y1 = ComponentHelpers::getBottomY(parent1);
         auto y2 = ComponentHelpers::getBottomY(parent2);

         if(y1 < y2)
            return true;
         if(y1 > y2)
            return false;           

         return parent1 < parent2;
      });

      for(auto &&de : eList)
         renderEntity(r, de);
   }
};


std::unique_ptr<RenderManager> buildRenderManager(int partitionIndex)
{
   return std::unique_ptr<RenderManager>(new RenderManagerImpl(partitionIndex));
}
