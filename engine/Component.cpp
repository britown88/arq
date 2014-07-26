//WaCkY CoMpOnEnT cPp FiLe
#include <vector>

#include "Component.h"

size_t componentCount;


void registerVTable(ComponentTypeVTable *vTable)
{
   vTables.push_back(vTable);
}

ComponentTypeVTable *getVTable(size_t index)
{
   return vTables[index];
}