#pragma once

#include "Entity.h"
#include "Component.h"
#include "Rect.h"
#include <vector>

struct PartitionSelectionComponent : public ComplexComponent
{
   PartitionSelectionComponent(){}
   PartitionSelectionComponent(int mailSlot):mailbox(mailSlot){}
   mutable int mailbox;
};

class PartitionManager : public IManager
{
public:
   virtual void getEntities(const Rectf &area, int selectionIndex)=0;
};

std::unique_ptr<PartitionManager> buildPartitionManager();

int getNewPartitionSelectionIndex();