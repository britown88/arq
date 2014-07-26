#pragma once

#include <memory>

class UIElement;
class GLWindow;

class MouseHandler
{
   class Impl;
   std::unique_ptr<Impl> pImpl;
public:
   MouseHandler(GLWindow *win);
   ~MouseHandler();

   void performMouseEvents(UIElement &rootElement);
   
   //removes focus from any currently focused element
   void resetMouseFocus();
};