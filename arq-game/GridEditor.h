#pragma once

#include "engine\UIElement.h"
#include "engine\CoreUI.h"
#include "engine\Entity.h"

//creates a uielement layer that captures mouse input to add and remove grid blocks
std::unique_ptr<UIElement> createGridEditor(CoreUI::WorldUIElement *parent, EntitySystem *system);