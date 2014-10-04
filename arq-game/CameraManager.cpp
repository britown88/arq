#include "CameraManager.h"
#include "engine\Component.h"
#include "ArqComponents.h"
#include "engine\IOCContainer.h"
#include "engine\Application.h"
#include "engine\CoreComponents.h"
#include "GridManager.h"

REGISTER_COMPONENT(CameraComponent);
REGISTER_COMPONENT(TargetComponent);

class CameraManagerImpl : public Manager<CameraManagerImpl, CameraManager>
{
   CoreUI::WorldUIElement *m_world;
public:
   CameraManagerImpl(CoreUI::WorldUIElement *world):m_world(world)
   {
   }

   static void registerComponentCallbacks(Manager<CameraManagerImpl, CameraManager> &m)
   {
   }

   void update()
   {
      if(auto c = getCameraEntity())
      {
         Float2 size;

         if(auto gbc = c->get<GraphicalBoundsComponent>())
            size = gbc->size;

         Rectf cam(0, 0, size.x, size.y);

         if(auto tc = c->get<TargetComponent>())
         {
            if(auto te = tc->target)
            if(auto tpos = te->get<PositionComponent>())
            {
               cam.offset(tpos->pos);

               if(auto cc = c->get<CenterComponent>())
                  cam.offset(-cc->center.x, -cc->center.y);
            }
         }
         else
         {

         }
         if(auto grid = m_system->getManager<GridManager>())
         {
            auto world = grid->getGridRect();

            if(cam.left < world.left) cam.offset(world.left - cam.left, 0.0f);
            if(cam.right > world.right) cam.offset(-(cam.right - world.right), 0.0f);
            if(cam.top < world.top) cam.offset(0.0f, world.top - cam.top);
            if(cam.bottom > world.bottom) cam.offset(0.0f, -(cam.bottom - world.bottom));


         }

                  
         m_world->cameraBounds() = cam;
      }
   }

   Entity *getCameraEntity()
   {
      auto &cameras = m_system->getComponentVector<CameraComponent>();
      if(!cameras.empty())
         return cameras[0].parent;

      return nullptr;
   }
};

std::unique_ptr<CameraManager> buildCameraManager(CoreUI::WorldUIElement *world)
{
   return std::unique_ptr<CameraManager>(new CameraManagerImpl(world));
}