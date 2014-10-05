#include "Actions.h"

#include "engine\CoreComponents.h"
#include "ArqComponents.h"
#include "GameData.h"
#include "engine\GameHelpers.h"
#include "engine\ComponentHelpers.h"

class Melee : public IAction
{
public:
   void buildSwordSlash(Entity *e, Direction dir)
   {
      auto vbo = GameHelpers::standardRectangleVBO();
      auto ibo = GameHelpers::standardRectangleIBO();


      float offset = 5.0f, width = 5.0f, length = 10.0f;
      auto swrd = e->getSystem()->createEntity();

      Float2 pos = e->get<PositionComponent>()->pos;
      Float2 size, center;
      switch (dir)
      {
      case Direction::Up:
         size = Float2(width, length);
         center = Float2(width/2.0f, length + offset);
         break;
      case Direction::Down:
         size = Float2(width, length);
         center = Float2(width/2.0f, -offset);
         break;
      case Direction::Left:
         size = Float2(length, width);
         center = Float2(length + offset, width/2.0f);
         break;
      case Direction::Right:
         size = Float2(length, width);
         center = Float2(-offset, width/2.0f);
         break;
      default:
         break;
      }

      swrd->add(PositionComponent(pos));
      swrd->add(GraphicalBoundsComponent(size));
      swrd->add(CenterComponent(center));
      swrd->add(CollisionBoxComponent(ComponentHelpers::getEntityRect(swrd)));
      swrd->add(MeshComponent(ibo, vbo, Colorf(1, 1, 1)));
      swrd->add(TimedLifeComponent(250.0f));
      swrd->add(AttackComponent(e, dir));      

      swrd->setNew();

   }

   void execute(Entity *e, Float2 target)
   {
      auto system = e->getSystem();
      if(auto ac = e->get<ActorComponent>())
      if(auto pc = e->get<PositionComponent>())
      {
         float horz = target.x - pc->pos.x;
         float vert = target.y - pc->pos.y;

         Direction dir;

         if(std::abs(horz) > std::abs(vert))
            dir = horz > 0.0f ? Direction::Right : Direction::Left;
         else
            dir = vert > 0.0f ? Direction::Down : Direction::Up;

         buildSwordSlash(e, dir);
      }
   }

   void end(Entity *e, Float2 target)
   {

   }
};

std::unique_ptr<IAction> Actions::buildMeleeAction()
{
   return std::unique_ptr<IAction>(new Melee());
}

