#include "UIDecorator.h"
#include "InputDefinitions.h"

class DraggableElement : public DecoratorUIElement
{
   bool m_dragging;
   Float2 m_clickPoint;
   Rectf m_startingBounds;
public:
   DraggableElement(std::unique_ptr<UIElement> inner)
      :DecoratorUIElement(std::move(inner)), m_dragging(false)
   {
      m_inner->registerMouseButton(Input::MouseLeft, Input::Press, 0, [&](MouseEvent e){onMouseDown((float)e.x, (float)e.y);});
      m_inner->registerMouseButton(Input::MouseLeft, Input::Release, 0, [&](MouseEvent e){onMouseUp((float)e.x, (float)e.y);});
      m_inner->registerMouseMove([&](MouseEvent e){onMouseMove((float)e.x, (float)e.y);});
   }

   void onMouseDown(float x, float y)
   {
      m_dragging = true;
      m_clickPoint = Float2(x, y);
      m_startingBounds = getBounds().absolute;
   }
   void onMouseUp(float x, float y)
   {
      m_dragging = false;
   }
   void onMouseMove(float x, float y)
   {
      if(m_dragging)
      {
         getBounds().absolute = m_startingBounds;
         getBounds().absolute.offset(Float2(x - m_clickPoint.x, y - m_clickPoint.y));
         if(auto parent = getParent())
            parent->arrange();
      }
   }
};

std::unique_ptr<UIElement> buildDraggableElement(std::unique_ptr<UIElement> inner)
{
   return std::unique_ptr<DraggableElement>(new DraggableElement(std::move(inner)));
}