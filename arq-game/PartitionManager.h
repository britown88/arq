#pragma once

#include "engine/Entity.h"
#include "engine/Component.h"
#include "engine/Rect.h"
#include <vector>

template<typename TAG>
class PartitionManager : public IManager
{
public:
   virtual std::vector<Entity*> getEntities(const Rectf &area)=0;
};

struct GraphicalTag{};
struct CollisionTag{};

typedef PartitionManager<GraphicalTag> GraphicalPartitionManager;
typedef PartitionManager<CollisionTag> CollisionPartitionManager;

std::unique_ptr<GraphicalPartitionManager> buildGraphicalPartitionManager();
std::unique_ptr<CollisionPartitionManager> buildCollisionPartitionManager();

