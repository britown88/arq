#include "RenderManager.h"
#include "PartitionManager.h"
#include "GridManager.h"

#include "engine/CoreComponents.h"
#include "engine/IOCContainer.h"
#include "engine/TextureManager.h"
#include "engine/Renderer.h"
#include "engine/ComponentHelpers.h"
#include "engine/Texture.h"
#include "engine/MatrixManager.h"
#include "engine/SeanSort.h"
#include "engine/SpriteManager.h"
#include "engine/Application.h"

struct TRenderComponent : public Component
{
   mutable double animStartTime;
   mutable ISpriteAnimation *sprite;
   mutable Texture *texture;
   mutable Matrix textureTransform;
   TRenderComponent():texture(nullptr), sprite(nullptr), animStartTime(0.0){}
};

REGISTER_COMPONENT(TRenderComponent)

struct DrawnEntity
{
   DrawnEntity(Entity *e, const Matrix *transform):e(e), transform(transform){}
   Entity *e;
   const Matrix *transform;
};

class RenderManagerImpl : public Manager<RenderManagerImpl, RenderManager>
{
   const Application &app;

   void renderEntity(Renderer &r, const DrawnEntity &de)
   {
      Texture *tex = nullptr;
      auto tc = de.e->get<TRenderComponent>();
      if(tc)
      {
         if(tc->sprite)
         {
            tex = tc->sprite->get((app.getTime() - tc->animStartTime));
         }
         else
         {
            tex = tc->texture;
         }         
      }

      auto mc = de.e->get<MeshComponent>();

      if(tex)
         r.drawTexture(mc->vbo, mc->ibo, tex, *de.transform, tc->textureTransform, mc->c);
      else
         r.drawTriangles(mc->vbo, mc->ibo, *de.transform, mc->c);
   }

public:
   RenderManagerImpl():app(*IOC.resolve<Application>()){}

   static void registerComponentCallbacks(Manager<RenderManagerImpl, RenderManager> &m)
   {
      m.add<TextureComponent>();
      m.add<SpriteComponent>();
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



   void onChanged(Entity *e, const SpriteComponent &oldData, const SpriteComponent &newData, int key)
   {      
      if(auto t = e->get<TRenderComponent>())
      if(oldData.sprite != newData.sprite)
      {
         t->sprite = IOC.resolve<SpriteManager>()->getSprite(newData.sprite);
         t->animStartTime = app.getTime();
      }
   }
   void onAdded(Entity *e, const SpriteComponent &comp, int key){}
   void onRemoved(Entity *e, const SpriteComponent &comp, int key){}

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

         if(auto sprite = e->get<SpriteComponent>())
         {
            trans.sprite = IOC.resolve<SpriteManager>()->getSprite(sprite->sprite);
            trans.animStartTime = app.getTime();
         }            

         e->add(trans);
      }
      
   }

   void onDelete(Entity *e)
   {
      e->remove<TRenderComponent>();
   }

   void draw(Renderer &r, Rectf drawBounds)
   {
      auto mm = m_system->getManager<MatrixManager>();
      if(!mm) return;

      std::vector<DrawnEntity> eList;
      ;


      for(auto &e : m_system->getManager<GraphicalPartitionManager>()->getEntities(drawBounds))
      {
         DrawnEntity de(e, mm->getMatrix(e));
         if(!de.transform)
            continue;

         eList.push_back(de);
      }

      for(auto &e : m_system->getManager<GridManager>()->collisionAt(drawBounds))
      {
         DrawnEntity de(e, mm->getMatrix(e));
         if(!de.transform)
            continue;

         eList.push_back(de);
      }

      eList = seanSort(std::move(eList), [&](const DrawnEntity &e1, const DrawnEntity &e2)->bool
      {

         //layering
         auto l1 = ComponentHelpers::getLayer(e1.e);
         auto l2 = ComponentHelpers::getLayer(e2.e);

         if(l1 < l2)
            return true;
         if(l1 > l2)
            return false; 

         //bottom Y
/*         auto y1 = ComponentHelpers::getBottomY(e1.e);
         auto y2 = ComponentHelpers::getBottomY(e2.e);

         if(y1 < y2)
            return true;
         if(y1 > y2)
            return false;   */        

         return e1.e < e2.e;
      });

      for(auto &&de : eList)
         renderEntity(r, de);
   }
};


std::unique_ptr<RenderManager> buildRenderManager()
{
   return std::unique_ptr<RenderManager>(new RenderManagerImpl());
}
