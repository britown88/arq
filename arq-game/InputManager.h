#pragma once

#include "engine\Entity.h"
#include "engine\UIElement.h"

struct PlayerControlledComponent : public Component{};

class InputManager : public IManager
{
public:
};

std::unique_ptr<InputManager> buildInputManager(UIElement *element);