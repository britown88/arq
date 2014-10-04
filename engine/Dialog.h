#pragma once

#include "UIElement.h"
#include <memory>

class Dialog
{
   class Impl;
   std::unique_ptr<Impl> pImpl;
public:
   Dialog(std::string title, float x, float y, float width, float height);
   ~Dialog();
   void setMaximized(bool maximized);
   void open(UIElement *parent);
   void close();
   UIElement *element();
   UIElement *outerElement();
};