#include "Tool.h"
#include "Dialog.h"
#include "UIElement.h"

#include <functional>

class UpdateElement : public UIElement
{
   typedef std::function<void()> UpdateFunc;
   UpdateFunc updateFunc;
   UIElementBounds m_bounds;
public:
   UpdateElement(UpdateFunc updateFunc):updateFunc(updateFunc){}
   void onStep(){updateFunc();}
   UIElementBounds &getBounds(){return m_bounds;}
};

void Tool::open(UIElement *parent)
{
   auto pos = getStartingPosition();
   auto size = getStartingSize();

   m_dlg.reset(new Dialog(getTitle(), pos.x, pos.y, size.x, size.y));
   auto updater = m_dlg->element()->intern(std::unique_ptr<UIElement>(new UpdateElement([&](){onStep();})));
   m_dlg->element()->pushListChild(updater);

   onOpen();

   m_dlg->open(parent);
}

void Tool::setMaximized(bool set)
{
   if(m_dlg)
      m_dlg->setMaximized(set);
}

UIElement *Tool::getDlgElement()
{
   return m_dlg ? m_dlg->element() : nullptr;
}

void Tool::close()
{
   onClose();
   m_dlg->close();
}