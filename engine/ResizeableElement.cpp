#include "UIDecorator.h"
#include "CoreUI.h"
#include "UISubsystem.h"
#include "InputDefinitions.h"

enum class ResizeEdge : unsigned int
{
   Top,
   Left,
   Right,
   Bottom,
   TopLeft,
   BottomLeft,
   TopRight,
   BottomRight
};

static const float BorderWidth = 10.0f;

class ResizeBorder : public DecoratorUIElement
{
 bool m_dragging;
   Float2 m_clickPoint;
   Rectf m_startingBounds;
   UIElement *m_resizable;//the parent
   ResizeEdge m_edge;
public:
   ResizeBorder(UIElement *resizable, ResizeEdge edge, std::unique_ptr<UIElement> inner)
      :DecoratorUIElement(std::move(inner)), m_resizable(resizable), m_edge(edge), m_dragging(false)
   {
      anchorToParent();

      switch(m_edge)
      {
      case ResizeEdge::Left:
         clearOption(UIOption::Right);
         setOption(UIOption::Width, BorderWidth);
         break;
      case ResizeEdge::Top:
         clearOption(UIOption::Bottom);
         setOption(UIOption::Height, BorderWidth);
         break;
      case ResizeEdge::Right:
         clearOption(UIOption::Left);
         setOption(UIOption::Width, BorderWidth);
         break;
      case ResizeEdge::Bottom:
         clearOption(UIOption::Top);
         setOption(UIOption::Height, BorderWidth);
         break;
      case ResizeEdge::TopLeft:
         clearOption(UIOption::Right);
         clearOption(UIOption::Bottom);
         setOption(UIOption::Width, BorderWidth);
         setOption(UIOption::Height, BorderWidth);
         break;
      case ResizeEdge::TopRight:
         clearOption(UIOption::Bottom);
         clearOption(UIOption::Left);
         setOption(UIOption::Width, BorderWidth);
         setOption(UIOption::Height, BorderWidth);
         break;
      case ResizeEdge::BottomLeft:
         clearOption(UIOption::Top);
         clearOption(UIOption::Right);
         setOption(UIOption::Width, BorderWidth);
         setOption(UIOption::Height, BorderWidth);
         break;
      case ResizeEdge::BottomRight:
         clearOption(UIOption::Top);
         clearOption(UIOption::Left);
         setOption(UIOption::Width, BorderWidth);
         setOption(UIOption::Height, BorderWidth);
         break;
      };

      m_inner->registerMouseButton(Input::MouseLeft, Input::Press, 0, [&](MouseEvent e){onMouseDown((float)e.x, (float)e.y);});
      m_inner->registerMouseButton(Input::MouseLeft, Input::Release, 0, [&](MouseEvent e){onMouseUp((float)e.x, (float)e.y);});
      m_inner->registerMouseMove([&](MouseEvent e){onMouseMove(e.x, e.y);});
   }

   void onMouseDown(float x, float y)
   {
      m_dragging = true;
      m_clickPoint = Float2(x, y);
      m_startingBounds = m_resizable->getBounds().absolute;
   }
   void onMouseUp(float x, float y)
   {
      m_dragging = false;
   }

   void onMouseMove(float x, float y)
   {
      if(m_dragging)
      {
         auto &b = m_resizable->getBounds();         
         b.absolute = m_startingBounds;
         
         switch(m_edge)
         {
         case ResizeEdge::Left:
            b.absolute.left += x - m_clickPoint.x;
            break;
         case ResizeEdge::Top:
            b.absolute.top += y - m_clickPoint.y;
            break;
         case ResizeEdge::Right:
            b.absolute.right += x - m_clickPoint.x;
            break;
         case ResizeEdge::Bottom:
            b.absolute.bottom += y - m_clickPoint.y;
            break;
         case ResizeEdge::TopLeft:
            b.absolute.top += y - m_clickPoint.y;
            b.absolute.left += x - m_clickPoint.x;
            break;
         case ResizeEdge::TopRight:
            b.absolute.top += y - m_clickPoint.y;
            b.absolute.right += x - m_clickPoint.x;
            break;
         case ResizeEdge::BottomLeft:
            b.absolute.bottom += y - m_clickPoint.y;
            b.absolute.left += x - m_clickPoint.x;
            break;
         case ResizeEdge::BottomRight:
            b.absolute.bottom += y - m_clickPoint.y;
            b.absolute.right += x - m_clickPoint.x;
            break;
         };

         if(auto p = m_resizable->getParent())
            p->arrange();         

      }
   }
};

std::unique_ptr<UIElement> buildResizableEdge(UIElement *resizable, ResizeEdge edge)
{
   return std::unique_ptr<ResizeBorder>(
      new ResizeBorder(resizable, edge, CoreUI::buildBasicElement(Rectf(), CoreUI::Layouts::buildFreeLayout())));
}


class ResizableElement : public SubsystemDecorator
{
   std::unique_ptr<UIElement> m_left, m_right, m_top, m_bottom, 
      m_topLeft, m_topRight, m_bottomLeft, m_bottomRight;

public:
   ResizableElement(std::unique_ptr<SubsystemElement> inner)
      :SubsystemDecorator(std::move(inner))
   {
      m_left = buildResizableEdge(this, ResizeEdge::Left);
      m_right = buildResizableEdge(this, ResizeEdge::Right);
      m_top = buildResizableEdge(this, ResizeEdge::Top);
      m_bottom = buildResizableEdge(this, ResizeEdge::Bottom);
      m_topLeft = buildResizableEdge(this, ResizeEdge::TopLeft);
      m_topRight = buildResizableEdge(this, ResizeEdge::TopRight);
      m_bottomLeft = buildResizableEdge(this, ResizeEdge::BottomLeft);
      m_bottomRight = buildResizableEdge(this, ResizeEdge::BottomRight);
   
      UIElement::pushListChild(m_left.get());
      UIElement::pushListChild(m_right.get());
      UIElement::pushListChild(m_top.get());
      UIElement::pushListChild(m_bottom.get());
      UIElement::pushListChild(m_topLeft.get());
      UIElement::pushListChild(m_topRight.get());
      UIElement::pushListChild(m_bottomLeft.get());
      UIElement::pushListChild(m_bottomRight.get());
   
   }

};

std::unique_ptr<SubsystemElement> buildResizableSubsystem(std::unique_ptr<SubsystemElement> inner)
{
   return std::unique_ptr<SubsystemElement>(new ResizableElement(std::move(inner)));
}