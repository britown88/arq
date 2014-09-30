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
      virtual Float2 vp2world(Float2 pos)=0;
   };
   std::unique_ptr<WorldUIElement> buildWorldUIElement(const Rectf &viewport, RenderManager *rm);
   std::unique_ptr<UIElement> buildBasicElement(const Rectf &bounds, std::unique_ptr<UILayout> layout);


   class TextElement : public UIElement
   {
   public:
      virtual void setText(std::string text)=0;
      virtual void setFont(std::shared_ptr<TextFont> font)=0;
      virtual void setColor(Colorf c)=0;
   };

   std::unique_ptr<TextElement> buildTextElement(std::string text, std::shared_ptr<TextFont> font, Float2 position = Float2());
   

   namespace Layouts
   {
      std::unique_ptr<UILayout> buildFreeLayout();
   };
};