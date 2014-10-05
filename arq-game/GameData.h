#pragma once

#include <unordered_map>
#include "engine\StringTable.h"
#include <memory>
#include "Actions.h"


enum class Direction : unsigned int
{
   Up = 0,
   Down,
   Left,
   Right
};

enum class ActionType : unsigned int
{
   MainHand = 0,
   OffHand
};

class IAction;

class GameData
{
public:
   std::unordered_map<InternString, std::unique_ptr<IAction>> actions;
};