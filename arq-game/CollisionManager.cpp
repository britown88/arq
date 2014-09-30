#include "CollisionManager.h"
#include "engine\Component.h"
#include "ArqComponents.h"

//REGISTER_COMPONENT(CollisionComponent);

struct TCollisionComponent : public Component
{
};

class CollisionManagerImpl : public Manager<CollisionManagerImpl, CollisionManager>
{

public:
   CollisionManagerImpl()
   {

   }

   ~CollisionManagerImpl()
   {
   }


   static void registerComponentCallbacks(Manager<CollisionManagerImpl, CollisionManager> &m)
   {
   }

   void update()
   {

   }

};

std::unique_ptr<CollisionManager> buildCollisionManager()
{
   return std::unique_ptr<CollisionManager>(new CollisionManagerImpl());
}