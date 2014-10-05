#include "Actions.h"

class Melee : public IAction
{
public:
   void execute(Entity *e, Float2 target)
   {
   }

   void end(Entity *e, Float2 target)
   {

   }
};

std::unique_ptr<IAction> Actions::buildMeleeAction()
{
   return std::unique_ptr<IAction>(new Melee());
}

