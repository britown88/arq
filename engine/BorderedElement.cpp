#include "UIDecorator.h"
#include "CoreUI.h"
#include "UISubsystem.h"

//what follows here is bullshit

class BorderedElement : public SubsystemDecorator
{
   std::unique_ptr<UIElement> m_left, m_right, m_top, m_bottom;

public:
   BorderedElement(std::unique_ptr<SubsystemElement> inner)
      :SubsystemDecorator(std::move(inner))
   {
      m_left = buildBackgroundedElement(CoreUI::buildBasicElement(Rectf(0, 0, 0, 0), CoreUI::Layouts::buildFreeLayout()));
      m_right = buildBackgroundedElement(CoreUI::buildBasicElement(Rectf(0, 0, 0, 0), CoreUI::Layouts::buildFreeLayout()));
      m_top = buildBackgroundedElement(CoreUI::buildBasicElement(Rectf(0, 0, 0, 0), CoreUI::Layouts::buildFreeLayout()));
      m_bottom = buildBackgroundedElement(CoreUI::buildBasicElement(Rectf(0, 0, 0, 0), CoreUI::Layouts::buildFreeLayout()));
   
      m_left->anchorToParent();
      m_left->clearOption(UIOption::Right);

      m_right->anchorToParent();
      m_right->clearOption(UIOption::Left);

      m_top->anchorToParent();
      m_top->clearOption(UIOption::Bottom);

      m_bottom->anchorToParent();
      m_bottom->clearOption(UIOption::Top);

      UIElement::pushListChild(m_left.get());
      UIElement::pushListChild(m_right.get());
      UIElement::pushListChild(m_top.get());
      UIElement::pushListChild(m_bottom.get());
   
   }

   void updateBorders()
   {
      Rectf abs = Rectf(0, 0, m_innerSystem->getBounds().relative.width(), m_innerSystem->getBounds().relative.height());

      if(auto color = m_innerSystem->getInnerOption(UIOption::BorderColor))
      {
         m_left->setOption(UIOption::BackgroundColor, *color);
         m_right->setOption(UIOption::BackgroundColor, *color);
         m_top->setOption(UIOption::BackgroundColor, *color);
         m_bottom->setOption(UIOption::BackgroundColor, *color);
      }
      else
      {
         m_left->clearOption(UIOption::BackgroundColor);
         m_right->clearOption(UIOption::BackgroundColor);
         m_top->clearOption(UIOption::BackgroundColor);
         m_bottom->clearOption(UIOption::BackgroundColor);
      }

      if(auto left = m_innerSystem->getInnerOption(UIOption::BorderLeft))
      {
         float w = boost::get<float>(*left);
         m_innerSystem->getContainer()->setOption(UIOption::Left, w);
         m_left->setOption(UIOption::Width, w);         
      }
      else
         m_left->setOption(UIOption::Width, 0.0f);  

      if(auto right = m_innerSystem->getInnerOption(UIOption::BorderRight))
      {
         float w = boost::get<float>(*right);
         m_innerSystem->getContainer()->setOption(UIOption::Right, w);
         m_right->setOption(UIOption::Width, w); 
      }
      else
         m_right->setOption(UIOption::Width, 0.0f); 

      if(auto top = m_innerSystem->getInnerOption(UIOption::BorderTop))
      {
         float w = boost::get<float>(*top);
         m_innerSystem->getContainer()->setOption(UIOption::Top, w);
         m_top->setOption(UIOption::Height, w); 
      }
      else
         m_top->setOption(UIOption::Height, 0.0f); 

      if(auto bot= m_innerSystem->getInnerOption(UIOption::BorderBottom))
      {
         float w = boost::get<float>(*bot);
         m_innerSystem->getContainer()->setOption(UIOption::Bottom, w);
         m_bottom->setOption(UIOption::Height, w); 
      }
      else
         m_bottom->setOption(UIOption::Height, 0.0f); 

   }

   virtual void arrange()
   {
      updateBorders();//give borders correct dims
      m_innerSystem->arrange();
   }
};

//std::unique_ptr<UIElement> buildBorderedElement(std::unique_ptr<UIElement> inner)
//{
//   return std::unique_ptr<BorderedElement>(new BorderedElement(std::move(inner)));
//}

std::unique_ptr<SubsystemElement> buildBorderedSubsystem(std::unique_ptr<SubsystemElement> inner)
{
   return std::unique_ptr<SubsystemElement>(new BorderedElement(std::move(inner)));
}