#pragma once

#include <memory>

class UIElement;
class GLWindow;

class KeyHandler
{
   class Impl;
   std::unique_ptr<Impl> pImpl;
public:
   KeyHandler(GLWindow *win);
   ~KeyHandler();

   void performKeyEvents(UIElement &focusedElement);
};