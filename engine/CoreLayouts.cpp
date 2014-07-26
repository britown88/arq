#include "CoreUI.h"

void updateElementWithPositionOptions(UIElement *elem, UIElement *owner)
{   
   float ownerPadLeft = 0.0f;
   float ownerPadTop = 0.0f;
   float ownerPadRight = 0.0f;
   float ownerPadBottom = 0.0f;

   float padLeft = 0.0f;
   float padTop = 0.0f;
   float padRight = 0.0f;
   float padBottom = 0.0f;

   float marginLeft = 0.0f;
   float marginTop = 0.0f;
   float marginRight = 0.0f;
   float marginBottom = 0.0f;

   boost::optional<float> anchorLeft;
   boost::optional<float> anchorTop;
   boost::optional<float> anchorRight;
   boost::optional<float> anchorBottom;

   boost::optional<float> width;
   boost::optional<float> height;

   bool centerX = false;
   bool centerY = false;

   if(auto var = owner->getOption(UIOption::PaddingLeft)) ownerPadLeft = boost::get<float>(*var);
   if(auto var = owner->getOption(UIOption::PaddingTop)) ownerPadTop = boost::get<float>(*var);
   if(auto var = owner->getOption(UIOption::PaddingRight)) ownerPadRight = boost::get<float>(*var);
   if(auto var = owner->getOption(UIOption::PaddingBottom)) ownerPadBottom = boost::get<float>(*var);

   if(auto var = elem->getOption(UIOption::PaddingLeft)) padLeft = boost::get<float>(*var);
   if(auto var = elem->getOption(UIOption::PaddingTop)) padTop = boost::get<float>(*var);
   if(auto var = elem->getOption(UIOption::PaddingRight)) padRight = boost::get<float>(*var);
   if(auto var = elem->getOption(UIOption::PaddingBottom)) padBottom = boost::get<float>(*var);

   if(auto var = elem->getOption(UIOption::MarginLeft)) marginLeft = boost::get<float>(*var);
   if(auto var = elem->getOption(UIOption::MarginTop)) marginTop = boost::get<float>(*var);
   if(auto var = elem->getOption(UIOption::MarginRight)) marginRight = boost::get<float>(*var);
   if(auto var = elem->getOption(UIOption::MarginBottom)) marginBottom = boost::get<float>(*var);

   if(auto var = elem->getOption(UIOption::Left)) anchorLeft = boost::get<float>(*var);
   if(auto var = elem->getOption(UIOption::Top)) anchorTop = boost::get<float>(*var);
   if(auto var = elem->getOption(UIOption::Right)) anchorRight = boost::get<float>(*var);
   if(auto var = elem->getOption(UIOption::Bottom)) anchorBottom = boost::get<float>(*var);

   if(auto var = elem->getOption(UIOption::Width)) width = boost::get<float>(*var);
   if(auto var = elem->getOption(UIOption::Height)) height = boost::get<float>(*var);

   if(auto var = elem->getOption(UIOption::CenterX)) centerX = boost::get<bool>(*var);
   if(auto var = elem->getOption(UIOption::CenterY)) centerY = boost::get<bool>(*var);

   auto &bounds = elem->getBounds();
   auto &ownerBounds = owner->getBounds(); 

   Rectf ownerRelative(ownerBounds.relative);
   ownerRelative.right -= ownerPadRight + ownerPadLeft;
   ownerRelative.bottom -= ownerPadBottom + ownerPadTop;
   ownerRelative.offset(Float2(ownerPadLeft, ownerPadTop));

   if(anchorLeft)
   {
      bounds.absolute.left = *anchorLeft;
      if(!anchorRight && width)
      {
         bounds.absolute.right = bounds.absolute.left + *width;
      }
   }

   if(anchorTop)
   {
      bounds.absolute.top = *anchorTop;
      if(!anchorBottom && height)
      {
         bounds.absolute.bottom =  bounds.absolute.top + *height;
      }
   }

   if(anchorRight)
   {
      bounds.absolute.right = ownerRelative.width() - *anchorRight;
      if(!anchorLeft && width)
      {
         bounds.absolute.left = bounds.absolute.right - *width;
      }
   }

   if(anchorBottom)
   {
      bounds.absolute.bottom = ownerRelative.height() - *anchorBottom;
      if(!anchorTop && height)
      {
         bounds.absolute.top = bounds.absolute.bottom - *height;
      }
   }

   if(centerX && !anchorLeft && !anchorRight)
   {
      float w =  width ? *width : bounds.absolute.width();

      bounds.absolute.left = (ownerRelative.width() - w) / 2.0f;
      bounds.absolute.right = (ownerRelative.width() + w) / 2.0f;
   }

   if(centerY && !anchorTop && !anchorBottom)
   {
      float h =  height ? *height : bounds.absolute.height();

      bounds.absolute.top = (ownerRelative.height() - h) / 2.0f;
      bounds.absolute.bottom = (ownerRelative.height() + h) / 2.0f;
   }

   //stop from having a 0 size rect
   if(bounds.absolute.width() < 1)
      bounds.absolute.right = bounds.absolute.left + 1;
   if(bounds.absolute.height() < 1)
      bounds.absolute.bottom = bounds.absolute.top + 1;

   bounds.relative = bounds.absolute;

   //bounds.relative.offset(Float2(ownerPadLeft, ownerPadTop));
   bounds.relative.offset(Float2(ownerRelative.left, ownerRelative.top));

   //stop from having a 0 size rect
   if(bounds.relative.width() < 1)
      bounds.relative.right = bounds.relative.left + 1;
   if(bounds.relative.height() < 1)
      bounds.relative.bottom = bounds.relative.top + 1;

   bounds.outerClipped = bounds.relative;

   Rectf ownerClip(ownerBounds.innerClipped);

   if(ownerClip.contains(bounds.outerClipped))
      bounds.outerClipped = ownerClip.intersection(bounds.outerClipped);      
   else
      bounds.outerClipped = Rectf();

   bounds.innerClipped = bounds.relative; 
   bounds.innerClipped.left += padLeft;
   bounds.innerClipped.top += padTop;
   bounds.innerClipped.right -= padRight;
   bounds.innerClipped.bottom -= padBottom;

   if(ownerClip.contains(bounds.innerClipped))
      bounds.innerClipped = ownerClip.intersection(bounds.innerClipped);      
   else
      bounds.innerClipped = Rectf();

   //stop from having a 0 size rect
   if(bounds.outerClipped.width() < 1)
      bounds.outerClipped.right = bounds.outerClipped.left + 1;
   if(bounds.outerClipped.height() < 1)
      bounds.outerClipped.bottom = bounds.outerClipped.top + 1;

   //stop from having a 0 size rect
   if(bounds.innerClipped.width() < 1)
      bounds.innerClipped.right = bounds.innerClipped.left + 1;
   if(bounds.innerClipped.height() < 1)
      bounds.innerClipped.bottom = bounds.innerClipped.top + 1;
}


class FreeLayout : public UIListLayout
{
   std::vector<UIElement *> m_children;
public:
   ~FreeLayout()
   {      
      for(auto child : m_children)
         child->setParent(nullptr);
   }

   std::vector<UIElement *> getChildren(){return m_children;}
   void removeChild(UIElement *element)
   {
      auto iter = std::find(m_children.begin(), m_children.end(), element);
      if(iter != m_children.end())
         m_children.erase(iter);
   }
   void arrange()
   {
      if(owner)
      { 
         for(auto child : m_children)
         {
            updateElementWithPositionOptions(child, owner);            
         }
         
      }

      for(auto child : m_children)
      {
         child->arrange();
      }

      
   }

   void push(UIElement *child){m_children.push_back(child);}
   void pop(){m_children.pop_back();}
   void insert(UIElement *child, size_t index){m_children.insert(m_children.begin() + index, child);}
   void erase(size_t index){m_children.erase(m_children.begin() + index);}
};

std::unique_ptr<UILayout> CoreUI::Layouts::buildFreeLayout()
{
   return std::unique_ptr<UILayout>(new FreeLayout());
}