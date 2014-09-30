#pragma once

#include "UIElement.h"

class DecoratorUIElement : public UIElement
{
protected:
   std::unique_ptr<UIElement> m_inner;
   
public:
   DecoratorUIElement(std::unique_ptr<UIElement> inner):
      m_inner(std::move(inner))
   {
      m_inner->setHighestDecorator(this);
   }

   virtual void setParent(UIElement *p)
   {
      m_inner->setInputParent(p);
      m_inner->setParent(p);
   }
   virtual void setHighestDecorator(UIElement *e){m_inner->setHighestDecorator(e);}
   virtual void setInputParent(UIElement *e){m_inner->setInputParent(e);}
   virtual UIElement *getParent(){return m_inner->getParent();}

   virtual UILayout *getLayout(){return m_inner->getLayout();}
   virtual void draw(Renderer &r){m_inner->draw(r);} 
   virtual void onStep(){m_inner->onStep();}   
   virtual UIElementBounds &getBounds(){return m_inner->getBounds();}

   virtual void registerMouseCallback(MouseEvent e, UIMouseCallback cb){m_inner->registerMouseCallback(e, cb);}
   virtual void unRegisterMouseCallback(MouseEvent e){m_inner->unRegisterMouseCallback(e);}
   virtual bool onMouseEvent(MouseEvent e){return m_inner->onMouseEvent(e);}
   virtual void giveMouseFocus(MouseHandler *handler){m_inner->giveMouseFocus(handler);}
   virtual void removeMouseFocus(){m_inner->removeMouseFocus();}

   virtual void registerKeyCallback(KeyEvent e, UIKeyCallback cb){m_inner->registerKeyCallback(e, cb);}
   virtual void unRegisterKeyCallback(KeyEvent e){m_inner->unRegisterKeyCallback(e);}
   virtual bool onKeyEvent(KeyEvent e){return m_inner->onKeyEvent(e);}

   virtual void registerControllerCallback(ControllerEvent e, UIControllerCallback cb){m_inner->registerControllerCallback(e, cb);}
   virtual void unRegisterControllerCallback(ControllerEvent e){m_inner->unRegisterControllerCallback(e);}
   virtual bool onControllerEvent(ControllerEvent e){return m_inner->onControllerEvent(e);}

   //UI Helpers
   virtual void registerMouseButton(int button, int action, int mods, UIMouseCallback cb){m_inner->registerMouseButton(button, action, mods, cb);}
   virtual void registerMouseMove(UIMouseCallback cb){m_inner->registerMouseMove(cb);}
   virtual void registerMouseScroll(int mods, UIMouseCallback cb){m_inner->registerMouseScroll(mods, cb);}
   virtual void registerMouseEnter(bool entered, UIMouseCallback cb){m_inner->registerMouseEnter(entered, cb);}
   virtual void unregisterMouseButton(int button, int action, int mods){m_inner->unregisterMouseButton(button, action, mods);}
   virtual void unregisterMouseMove(){m_inner->unregisterMouseMove();}
   virtual void unregisterMouseScroll(int mods){m_inner->unregisterMouseScroll(mods);}
   virtual void unregisterMouseEnter(bool entered){m_inner->unregisterMouseEnter(entered);}

   virtual void registerKeyboardKey(int key, int action, int mods, UIKeyCallback cb){m_inner->registerKeyboardKey(key, action, mods, cb);}
   virtual void unregisterKeyboardKey(int key, int action, int mods){m_inner->unregisterKeyboardKey(key, action, mods);}

   virtual void registerControllerPresence(bool attached, UIControllerCallback cb){m_inner->registerControllerPresence(attached, cb);}
   virtual void registerControllerButton(int id, int button, int action, UIControllerCallback cb){m_inner->registerControllerButton(id, button, action, cb);}
   virtual void registerControllerAxis(int id, int axis, int action, UIControllerCallback cb){m_inner->registerControllerAxis(id, axis, action, cb);}
   virtual void unregisterControllerPresence(bool attached){m_inner->unregisterControllerPresence(attached);}
   virtual void unregisterControllerButton(int id, int button, int action){m_inner->unregisterControllerButton(id, button, action);}
   virtual void unregisterControllerAxis(int id, int axis, int action){m_inner->unregisterControllerAxis(id, axis, action);}

   //helpers
   virtual boost::optional<UIOptionVar> getOption(UIOption opt){return m_inner->getOption(opt);}
   virtual void setOption(UIOption opt, UIOptionVar var){m_inner->setOption(opt, var);}
   virtual void clearOption(UIOption opt){m_inner->clearOption(opt);}
   //virtual void clearAllOptions(){m_inner->clearAllOptions();}
   virtual void setOptionAllBorders(float width){m_inner->setOptionAllBorders(width);}
   virtual void setOptionAllMargins(float width){m_inner->setOptionAllMargins(width);}
   virtual void setOptionAllPadding(float width){m_inner->setOptionAllPadding(width);}
   virtual void anchorToParentWidth(float distance = 0.0f){m_inner->anchorToParentWidth(distance);}
   virtual void anchorToParentHeight(float distance = 0.0f){m_inner->anchorToParentHeight(distance);}
   virtual void anchorToParent(float distance = 0.0f){m_inner->anchorToParent(distance);}
   virtual void setOptionAnchors(float left, float top, float right, float bottom){m_inner->setOptionAnchors(left, top, right, bottom);}
   virtual void clearAnchors(){m_inner->clearAnchors();}

   virtual void pushGridChild(UIElement *elem, size_t column){m_inner->pushGridChild(elem, column);}
   virtual void popGridChild(size_t column){m_inner->popGridChild(column);}
   virtual void insertGridChild(UIElement *elem, size_t index, size_t column){m_inner->insertGridChild(elem, index, column);}
   virtual void eraseGridChild(size_t index, size_t column){m_inner->eraseGridChild(index, column);}
   virtual void pushListChild(UIElement *elem){m_inner->pushListChild(elem);}
   virtual void popListChild(){m_inner->popListChild();}
   virtual void insertListChild(UIElement *elem, size_t index){m_inner->insertListChild(elem, index);}
   virtual void eraseListChild(size_t index){m_inner->eraseListChild(index);}

   virtual void update(){m_inner->update();}
   virtual void arrange(){m_inner->arrange();}
};


std::unique_ptr<UIElement> buildDraggableElement(std::unique_ptr<UIElement> inner);
std::unique_ptr<UIElement> buildBackgroundedElement(std::unique_ptr<UIElement> inner);
//std::unique_ptr<UIElement> buildBorderedElement(std::unique_ptr<UIElement> inner);
//std::unique_ptr<UIElement> buildResizableElement(std::unique_ptr<UIElement> inner);