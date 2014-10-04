#include "CameraManager.h"
#include "engine\Component.h"
#include "ArqComponents.h"
#include "engine\IOCContainer.h"
#include "engine\Application.h"
#include "engine\CoreComponents.h"

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

         if(auto tc = c->get<TargetComponent>())
         if(auto te = tc->target)
         if(auto tpos = te->get<PositionComponent>())
         {
            Rectf cam(0, 0, size.x, size.y);
            cam.offset(tpos->pos);
            cam.offset(-size.x/2, -size.y/2);
            m_world->cameraBounds() = cam;
         }
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