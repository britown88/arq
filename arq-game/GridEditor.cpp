#include "GridEditor.h"
#include "GridManager.h"

#include "engine\InputDefinitions.h"
#include "engine\CoreComponents.h"
#include "engine\GameHelpers.h"
#include "engine\IOCContainer.h"
#include "engine\Application.h"

#include "engine\Logs.h"

class GridEditorElement : public UIElement
{
   UIElementBounds m_bounds;
   EntitySystem *m_system;
   CoreUI::WorldUIElement *m_parent;

   struct GridEntity
   {
      Int2 pos;
      Entity *e;
   };

   GridEntity entityAt(float x, float y)
   {
      auto gm = m_system->getManager<GridManager>();

      GridEntity ge;
      ge.pos = gm->gridPosition(m_parent->vp2world(Float2(x, y)));
      ge.e = gm->entityAt(ge.pos);

      return ge;
   };

   bool leftDown, rightDown;

   void buildBlock(Recti cellRange)
   {
      auto app = IOC.resolve<Application>();

      auto vbo = GameHelpers::standardRectangleVBO();
      auto ibo = GameHelpers::standardRectangleIBO();

      auto st = IOC.resolve<StringTable>();

      auto e = m_system->createEntity();
      e->add(PositionComponent());
      e->add(GraphicalBoundsComponent());
      e->add(CollisionBoxComponent());
      e->add(MeshComponent(ibo, vbo, Colorf(1, 1, 1)));
      e->add(TextureComponent(st->get("assets/img/block.png")));
      e->add(GridComponent(cellRange));

      e->setNew();

   }

   void createAt(float x, float y)
   {
      auto e = entityAt(x, y);

      if(!e.e)
      {
         Logs::d("GridEditor") << "Create block at (" << e.pos.x << ", " << e.pos.y << ")";
         buildBlock(Recti(e.pos.x, e.pos.y, e.pos.x+1, e.pos.y+1));
      }
         
   }

   void deleteAt(float x, float y)
   {
      auto e = entityAt(x, y);

      if(e.e)
      {
         Logs::w("GridEditor") << "Delete block at (" << e.pos.x << ", " << e.pos.y << ")";
         e.e->setDeleted();
      }

   }

public:
   GridEditorElement(CoreUI::WorldUIElement *parent, EntitySystem *system)
      :m_parent(parent), m_system(system), leftDown(false), rightDown(false)
   {
      registerMouseMove([=](MouseEvent e){onMove((float)e.x, (float)e.y);});
      registerMouseButton(Input::MouseLeft, Input::Press, Input::ModControl, [=](MouseEvent e){onLeftDown((float)e.x, (float)e.y);});
      registerMouseButton(Input::MouseLeft, Input::Release, 0, [=](MouseEvent e){onLeftUp((float)e.x, (float)e.y);});
      registerMouseButton(Input::MouseRight, Input::Press, Input::ModControl, [=](MouseEvent e){onRightDown((float)e.x, (float)e.y);});
      registerMouseButton(Input::MouseRight, Input::Release, 0, [=](MouseEvent e){onRightUp((float)e.x, (float)e.y);});
   }
   ~GridEditorElement()
   {
   }

   void onMove(float x, float y)
   {
      if(leftDown)
         createAt(x, y);
      if(rightDown)
         deleteAt(x, y);
   }

   void onLeftDown(float x, float y)
   {
      leftDown = true;
      createAt(x, y);
   }

   void onLeftUp(float x, float y)
   {
      leftDown = false;
   }

   void onRightDown(float x, float y)
   {
      rightDown = true;
      deleteAt(x, y);
   }

   void onRightUp(float x, float y)
   {
      rightDown = false;
   }

   UIElementBounds &getBounds(){return m_bounds;}
};

std::unique_ptr<UIElement> createGridEditor(CoreUI::WorldUIElement *parent, EntitySystem *system)
{
   auto elem = new GridEditorElement(parent, system);

   parent->pushListChild(elem);
   elem->anchorToParent();
   elem->arrange();

   return std::unique_ptr<UIElement>(elem);

}