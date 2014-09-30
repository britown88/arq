#pragma once

#include "Renderer.h"
#include "Rect.h"
#include "MouseEvent.h"
#include "KeyEvent.h"
#include "ControllerEvent.h"
#include "ObjectHash.h"
#include <functional>
#include <unordered_map>
#include <memory>
#include "UIOptions.h"

#include "boost\optional.hpp"

class UIListLayout;
class UIGridLayout;
class UIElement;

class UILayout
{
public:
   virtual ~UILayout(){}
   virtual std::vector<UIElement *> getChildren()=0;
   virtual void removeChild(UIElement *element)=0;
   virtual void arrange()=0;

   virtual UIListLayout *getListLayout(){return nullptr;}
   virtual UIGridLayout *getGridLayout(){return nullptr;}

   UIElement *owner;
};

class UIListLayout : public UILayout
{
public:
   virtual void push(UIElement *child)=0;
   virtual void pop()=0;
   virtual void insert(UIElement *child, size_t index)=0;
   virtual void erase(size_t index)=0;

   UIListLayout *getListLayout(){return this;}
};

class UIGridLayout : public UILayout
{
public:
   virtual void push(UIElement *child, size_t column)=0;
   virtual void pop(size_t column)=0;
   virtual void insert(UIElement *child, size_t index, size_t column)=0;
   virtual void erase(size_t index, size_t column)=0;

   UIGridLayout *getGridLayout(){return this;}
};

struct UIElementBounds
{
   //absolute dimensions of element
   //These can be changed and wil factor the layout
   //options in the element may set these during the layout's arrange
   Rectf absolute;

   //do not set these outside of the layout!

   //mpAbsolute modified by parent element position
   Rectf relative;

   //this.relative clipped to be within parent bounds
   Rectf innerClipped;

   //this.innerClipped except constrained for children (ie for padding)
   Rectf outerClipped;
};

typedef std::function<void(MouseEvent &)> UIMouseCallback;
typedef std::function<void(KeyEvent &)> UIKeyCallback;
typedef std::function<void(ControllerEvent &)> UIControllerCallback;

class MouseHandler;
class Application;

//All members must be virtual and added to UIDecoratorElement
//calling m_inner's.  Decorators can then just implement UIDecoratorElement
//and not have to c+p everything in here
class UIElement 
{   
   std::unordered_map<MouseEvent, 
      UIMouseCallback, ObjectHash<MouseEvent>> m_mouseCallbacks;

   std::unordered_map<KeyEvent, 
      UIKeyCallback, ObjectHash<KeyEvent>> m_keyCallbacks;

   std::unordered_map<ControllerEvent, 
      UIControllerCallback, ObjectHash<ControllerEvent>> m_controllerCallbacks;

   MouseHandler *m_mouseFocusHandler;
   boost::optional<UIOptionVar> m_options[UIOption::COUNT];

   std::vector<std::unique_ptr<UIElement>> m_internElements;
   UIElement *m_highestDecorator, *m_inputParent;
   Application *m_app;
protected:
   //overload this in decorators to ensure changes to the parent get reflected all the way down
   
   UIElement *m_parent;
public:
   template<typename T>
   T *intern(std::unique_ptr<T> elem)
   {
      auto out = elem.get();
      m_internElements.push_back(std::move(elem));

      return out;
   }

   UIElement();
   virtual ~UIElement();
   virtual UILayout *getLayout(){return nullptr;}
   virtual void draw(Renderer &r){} 
   virtual void onStep(){}   
   virtual UIElementBounds &getBounds()=0;

   virtual UIElement *getParent();
   virtual void setParent(UIElement *p);
   virtual void setHighestDecorator(UIElement *e);
   virtual void setInputParent(UIElement *e);


   //Mouse
   virtual void registerMouseCallback(MouseEvent e, UIMouseCallback cb);
   virtual void unRegisterMouseCallback(MouseEvent e);
   virtual bool onMouseEvent(MouseEvent e);
   virtual void giveMouseFocus(MouseHandler *handler);
   virtual void removeMouseFocus();

   //keys
   virtual void registerKeyCallback(KeyEvent e, UIKeyCallback cb);
   virtual void unRegisterKeyCallback(KeyEvent e);
   virtual bool onKeyEvent(KeyEvent e);

   //controller
   virtual void registerControllerCallback(ControllerEvent e, UIControllerCallback cb);
   virtual void unRegisterControllerCallback(ControllerEvent e);
   virtual bool onControllerEvent(ControllerEvent e);

   //UI Helpers
   virtual void registerMouseButton(int button, int action, int mods, UIMouseCallback cb);
   virtual void registerMouseMove(UIMouseCallback cb);
   virtual void registerMouseScroll(int mods, UIMouseCallback cb);
   virtual void registerMouseEnter(bool entered, UIMouseCallback cb);   
   virtual void unregisterMouseButton(int button, int action, int mods);
   virtual void unregisterMouseMove();
   virtual void unregisterMouseScroll(int mods);
   virtual void unregisterMouseEnter(bool entered);

   virtual void registerKeyboardKey(int key, int action, int mods, UIKeyCallback cb);
   virtual void unregisterKeyboardKey(int key, int action, int mods);

   virtual void registerControllerPresence(bool attached, UIControllerCallback cb);
   virtual void registerControllerButton(int id, int button, int action, UIControllerCallback cb);
   virtual void registerControllerAxis(int id, int axis, int action, UIControllerCallback cb);
   virtual void unregisterControllerPresence(bool attached);
   virtual void unregisterControllerButton(int id, int button, int action);
   virtual void unregisterControllerAxis(int id, int axis, int action);

   //helpers
   virtual boost::optional<UIOptionVar> getOption(UIOption opt);
   virtual void setOption(UIOption opt, UIOptionVar var);
   virtual void clearOption(UIOption opt);
   
   //getting rid of this because dont fucking do this
   //virtual void clearAllOptions();

   virtual void setOptionAllBorders(float width);
   virtual void setOptionAllMargins(float width);
   virtual void setOptionAllPadding(float width);
   virtual void setOptionAnchors(float left, float top, float right, float bottom);
   virtual void anchorToParentWidth(float distance = 0.0f);
   virtual void anchorToParentHeight(float distance = 0.0f);
   virtual void anchorToParent(float distance = 0.0f);
   virtual void clearAnchors();


   virtual void update();//calls step on self and children
   virtual void pushGridChild(UIElement *elem, size_t column);
   virtual void popGridChild(size_t column);
   virtual void insertGridChild(UIElement *elem, size_t index, size_t column);
   virtual void eraseGridChild(size_t index, size_t column);

   virtual void pushListChild(UIElement *elem);
   virtual void popListChild();
   virtual void insertListChild(UIElement *elem, size_t index);
   virtual void eraseListChild(size_t index);

   virtual void arrange();
};


