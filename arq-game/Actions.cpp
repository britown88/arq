#include "Actions.h"

class MeleeWeapon : public IAction
{
public:
   void execute(Entity *e, ActionType type, Float2 target)
   {

   }

   void end(Entity *e, ActionType type, Float2 target)
   {

   }
};

std::unique_ptr<IAction> buildMeleeWeaponAction(Property &p)
{
   return std::unique_ptr<IAction>(new MeleeWeapon());
}

