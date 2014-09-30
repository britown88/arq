#pragma once

#include <memory>

class UIElement;
class GLWindow;

class ControllerHandler
{
   class Impl;
   std::unique_ptr<Impl> pImpl;
public:
   ControllerHandler(GLWindow *win);
   ~ControllerHandler();

   void performControllerEvents(UIElement &focusedElement);
};