#include "UIElement.h"
#include "MouseHandler.h"
#include "IOCContainer.h"
#include "Application.h"

UIElement::UIElement()
   :m_parent(nullptr), m_mouseFocusHandler(nullptr), m_highestDecorator(this), m_inputParent(nullptr),
   m_app(&(*IOC.resolve<Application>()))
{
}

UIElement::~UIElement()
{
   m_app->removeFocus(this);

   if(m_mouseFocusHandler)
      m_mouseFocusHandler->resetMouseFocus();

   if(m_parent)
      if(auto children = m_parent->getLayout())
         children->removeChild(m_highestDecorator);
}

void UIElement::setParent(UIElement *p)
{
   m_parent = p;
}
void UIElement::setHighestDecorator(UIElement *e){m_highestDecorator = e;}
void UIElement::setInputParent(UIElement *e){m_inputParent = e;}
UIElement *UIElement::getParent()
{
   return m_parent;
}

void UIElement::update()
{
   onStep();
   if(auto l = getLayout())
   {
      for(auto child : l->getChildren())
         child->update();
   }
}

void UIElement::registerMouseCallback(MouseEvent e, UIMouseCallback cb)
{
   m_mouseCallbacks.insert(std::make_pair(e, cb));
}

void UIElement::unRegisterMouseCallback(MouseEvent e)
{
   m_mouseCallbacks.erase(e);
}

bool UIElement::onMouseEvent(MouseEvent e)
{
   auto iter = m_mouseCallbacks.find(e);
   if(iter != m_mouseCallbacks.end())
   {
      if(m_inputParent)
      {
         e.x -= m_inputParent->getBounds().relative.left;
         e.y -= m_inputParent->getBounds().relative.top;
      }

      iter->second(e);
      return true;
   }

   return false;         
}

void UIElement::registerKeyCallback(KeyEvent e, UIKeyCallback cb){m_keyCallbacks.insert(std::make_pair(e, cb));}
void UIElement::unRegisterKeyCallback(KeyEvent e){m_keyCallbacks.erase(e);}
bool UIElement::onKeyEvent(KeyEvent e)
{
   auto iter = m_keyCallbacks.find(e);
   if(iter != m_keyCallbacks.end())
   {
      iter->second(e);
      return true;
   }

   return false;  
}

void UIElement::registerControllerCallback(ControllerEvent e, UIControllerCallback cb){m_controllerCallbacks.insert(std::make_pair(e, cb));}
void UIElement::unRegisterControllerCallback(ControllerEvent e){m_controllerCallbacks.erase(e);}
bool UIElement::onControllerEvent(ControllerEvent e)
{
   auto iter = m_controllerCallbacks.find(e);
   if(iter != m_controllerCallbacks.end())
   {
      iter->second(e);
      return true;
   }

   return false;  
}

void UIElement::giveMouseFocus(MouseHandler *handler)
{
   m_mouseFocusHandler = handler;
}
void UIElement::removeMouseFocus()
{
   m_mouseFocusHandler = nullptr;
}

void UIElement::registerKeyboardKey(int key, int action, int mods, UIKeyCallback cb)
{
   KeyEvent e(KeyEvent::Type::Key);
   e.key = key;
   e.action = action;
   e.mods = mods;
   registerKeyCallback(e, cb);
}
void UIElement::unregisterKeyboardKey(int key, int action, int mods)
{
   KeyEvent e(KeyEvent::Type::Key);
   e.key = key;
   e.action = action;
   e.mods = mods;
   unRegisterKeyCallback(e);
}

void UIElement::registerControllerPresence(bool attached, UIControllerCallback cb)
{
   ControllerEvent ce(ControllerEvent::Type::Presence);
   ce.attached = attached;
   registerControllerCallback(ce, cb);
}
void UIElement::registerControllerButton(int id, int button, int action, UIControllerCallback cb)
{
   ControllerEvent ce(ControllerEvent::Type::Button);
   ce.id = id;
   ce.buttonaxis = button;
   ce.action = action;
   registerControllerCallback(ce, cb);
}
void UIElement::registerControllerAxis(int id, int axis, int action, UIControllerCallback cb)
{
   ControllerEvent ce(ControllerEvent::Type::Axis);
   ce.id = id;
   ce.buttonaxis = axis;
   ce.action = action;
   registerControllerCallback(ce, cb);
}
void UIElement::unregisterControllerPresence(bool attached)
{
   ControllerEvent ce(ControllerEvent::Type::Presence);
   ce.attached = attached;
   unRegisterControllerCallback(ce);
}
void UIElement::unregisterControllerButton(int id, int button, int action)
{
   ControllerEvent ce(ControllerEvent::Type::Button);
   ce.id = id;
   ce.buttonaxis = button;
   ce.action = action;
   unRegisterControllerCallback(ce);
}
void UIElement::unregisterControllerAxis(int id, int axis, int action)
{
   ControllerEvent ce(ControllerEvent::Type::Axis);
   ce.id = id;
   ce.buttonaxis = axis;
   ce.action = action;
   unRegisterControllerCallback(ce);
}

void UIElement::registerMouseButton(int button, int action, int mods, UIMouseCallback cb)
{
   MouseEvent e(MouseEvent::Type::Button);
   e.button = button;
   e.action = action;
   e.mods = mods;
   registerMouseCallback(e, cb);
}

void UIElement::registerMouseMove(UIMouseCallback cb)
{
   MouseEvent e(MouseEvent::Type::Move);
   registerMouseCallback(e, cb);
}

void UIElement::registerMouseScroll(int mods, UIMouseCallback cb)
{
   MouseEvent e(MouseEvent::Type::Scroll);
   e.mods = mods;
   registerMouseCallback(e, cb);
}

void UIElement::registerMouseEnter(bool entered, UIMouseCallback cb)
{
   MouseEvent e(MouseEvent::Type::Enter);
   e.entered = entered;
   registerMouseCallback(e, cb);
}

void UIElement::unregisterMouseButton(int button, int action, int mods)
{
   MouseEvent e(MouseEvent::Type::Button);
   e.button = button;
   e.action = action;
   e.mods = mods;
   unRegisterMouseCallback(e);
}

void UIElement::unregisterMouseMove()
{
   MouseEvent e(MouseEvent::Type::Move);
   unRegisterMouseCallback(e);
}

void UIElement::unregisterMouseScroll(int mods)
{
   MouseEvent e(MouseEvent::Type::Scroll);
   e.mods = mods;
   unRegisterMouseCallback(e);
}

void UIElement::unregisterMouseEnter(bool entered)
{
   MouseEvent e(MouseEvent::Type::Enter);
   e.entered = entered;
   unRegisterMouseCallback(e);
}

boost::optional<UIOptionVar> UIElement::getOption(UIOption opt)
{
   return m_options[(unsigned int)opt];
}

void UIElement::setOption(UIOption opt, UIOptionVar var)
{
   m_options[(unsigned int)opt] = var;
}

void UIElement::clearOption(UIOption opt)
{
   m_options[(unsigned int)opt] = boost::none;
}

//void UIElement::clearAllOptions()
//{
//   for(unsigned int i = 0; i < (unsigned int)UIOption::COUNT; ++i)
//      m_options[i] = boost::none;
//}

void UIElement::setOptionAllBorders(float width)
{
   setOption(UIOption::BorderBottom, width);
   setOption(UIOption::BorderTop, width);
   setOption(UIOption::BorderLeft, width);
   setOption(UIOption::BorderRight, width);
}

void UIElement::setOptionAllMargins(float width)
{
   setOption(UIOption::MarginBottom, width);
   setOption(UIOption::MarginTop, width);
   setOption(UIOption::MarginLeft, width);
   setOption(UIOption::MarginRight, width);
}

void UIElement::setOptionAllPadding(float width)
{
   setOption(UIOption::PaddingBottom, width);
   setOption(UIOption::PaddingTop, width);
   setOption(UIOption::PaddingLeft, width);
   setOption(UIOption::PaddingRight, width);
}

void UIElement::setOptionAnchors(float left, float top, float right, float bottom)
{
   setOption(UIOption::Left, left);
   setOption(UIOption::Right, right);
   setOption(UIOption::Top, top);
   setOption(UIOption::Bottom, bottom);
}

void UIElement::anchorToParentWidth(float distance)
{
   setOption(UIOption::Left, distance);
   setOption(UIOption::Right, distance);
}

void UIElement::anchorToParentHeight(float distance)
{
   setOption(UIOption::Top, distance);
   setOption(UIOption::Bottom, distance);
}

void UIElement::anchorToParent(float distance)
{
   anchorToParentWidth(distance);
   anchorToParentHeight(distance);
}

void UIElement::clearAnchors()
{
   clearOption(UIOption::Left);
   clearOption(UIOption::Top);
   clearOption(UIOption::Right);
   clearOption(UIOption::Bottom);
}

void UIElement::pushGridChild(UIElement *elem, size_t column)
{
   if(auto l = getLayout())
      if(auto gl = l->getGridLayout())
      {
         gl->push(elem, column);
         elem->setParent(this);
      } 
}

void UIElement::insertGridChild(UIElement *elem, size_t index, size_t column)
{
   if(auto l = getLayout())
      if(auto gl = l->getGridLayout())
      {
         gl->insert(elem, index, column);
         elem->setParent(this);
      }
}

void UIElement::eraseGridChild(size_t index, size_t column)
{
   if(auto l = getLayout())
      if(auto gl = l->getGridLayout())
         gl->erase(index, column);
}

void UIElement::popGridChild(size_t column)
{
   if(auto l = getLayout())
      if(auto gl = l->getGridLayout())
         gl->pop(column);
}

void UIElement::pushListChild(UIElement *elem)
{
   if(auto l = getLayout())
      if(auto ll = l->getListLayout())
      {
         ll->push(elem);
         elem->setParent(this);
      }
}

void UIElement::insertListChild(UIElement *elem, size_t index)
{
   if(auto l = getLayout())
      if(auto ll = l->getListLayout())
      {
         ll->insert(elem, index);
         elem->setParent(this);
      }
}

void UIElement::eraseListChild(size_t index)
{
   if(auto l = getLayout())
      if(auto ll = l->getListLayout())
         ll->erase(index);
}

void UIElement::popListChild()
{
   if(auto l = getLayout())
      if(auto ll = l->getListLayout())
         ll->pop();
}

void UIElement::arrange()
{
   if(auto l = getLayout())
      l->arrange();
}