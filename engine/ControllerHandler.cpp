#include "ControllerHandler.h"
#include "GLWindow.h"
#include "UIElement.h"
#include "InputDefinitions.h"
#include "ControllerEvent.h"

#include <vector>

class ControllerHandler::Impl
{
   std::vector<ControllerEvent> m_events;

   int m_focusedButton;

   ControllerHandler *parent;
   GLWindow *m_window;

public:
   Impl(GLWindow *win, ControllerHandler *parent):m_window(win), parent(parent)
   {
      win->setControllerPresenceCallback([&](int id, bool attached){onPresence(id, attached);});
      win->setControllerButtonCallback([&](int id, int button, int action){onButton(id, button, action);});
      win->setControllerAxisCallback([&](int id, int axis, int action){onAxis(id, axis, action);});

   }
   ~Impl(){}

   void onPresence(int id, bool attached)
   {
      ControllerEvent ke(ControllerEvent::Type::Presence);
      ke.id = id;
      ke.attached = attached;
      m_events.push_back(ke);
   }

   void onButton(int id, int button, int action)
   {
      ControllerEvent ke(ControllerEvent::Type::Button);
      ke.id = id;
      ke.buttonaxis = button;
      ke.action = action;
      m_events.push_back(ke);
   }

   void onAxis(int id, int axis, int action)
   {
      ControllerEvent ke(ControllerEvent::Type::Axis);
      ke.id = id;
      ke.buttonaxis = axis;
      ke.action = action;
      m_events.push_back(ke);
   }

   void performControllerEvents(UIElement &focusedElement)
   {
      for(auto &event : m_events)
      {
         auto e = &focusedElement;
         while(e)
         {
            if(e->onControllerEvent(event))
               break;
            e = e->getParent();
         }
      }

      m_events.clear();
      
   }
};

ControllerHandler::ControllerHandler(GLWindow *win)
   :pImpl(new Impl(win, this)){}
ControllerHandler::~ControllerHandler(){}
void ControllerHandler::performControllerEvents(UIElement &focusedElement){pImpl->performControllerEvents(focusedElement);}