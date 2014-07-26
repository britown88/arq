#pragma once

#include "UIElement.h"
#include "Text.h"

class RenderManager;

namespace CoreUI
{   
   std::unique_ptr<UIElement> buildRootUIElement();

   class WorldUIElement : public UIElement
   {
   public:
      virtual Rectf &cameraBounds()=0;
   };
   std::unique_ptr<WorldUIElement> buildWorldUIElement(const Rectf &viewport, RenderManager *rm);
   std::unique_ptr<UIElement> buildBasicElement(const Rectf &bounds, std::unique_ptr<UILayout> layout);
   std::unique_ptr<UIElement> buildTextElement(std::string text, std::shared_ptr<TextFont> font, Float2 position = Float2());
   

   namespace Layouts
   {
      std::unique_ptr<UILayout> buildFreeLayout();
   };
};