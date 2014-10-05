#pragma once

#include "engine\Entity.h"
#include "engine\UIElement.h"
#include "engine\CoreUI.h"

struct PlayerControlledComponent : public Component{};

class InputManager : public IManager
{
public:
};

std::unique_ptr<InputManager> buildInputManager(CoreUI::WorldUIElement *element);