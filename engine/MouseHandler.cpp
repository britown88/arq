#include "MouseHandler.h"
#include "GLWindow.h"
#include "MouseEvent.h"
#include "UIElement.h"
#include "InputDefinitions.h"

#include <vector>

class MouseHandler::Impl
{
   std::vector<MouseEvent> m_events;

   UIElement *m_focusedElement;
   int m_focusedButton;

   MouseHandler *parent;
   GLWindow *m_window;

public:
   Impl(GLWindow *win, MouseHandler *parent):m_window(win), m_focusedElement(nullptr), parent(parent)
   {
      win->setMouseButtonCallback([&](int button, int action, int mods, double x, double y){onButton(button, action, mods, x, y);});
      win->setMousePositionCallback([&](double x, double y){onMove(x, y);});
      win->setMouseEnterCallback([&](bool entered, double x, double y){onEnter(entered, x, y);});
      win->setMouseScrollCallback([&](double x, double y, int mods, double px, double py){onScroll(x, y, mods, px, py);});
   }
   ~Impl(){}

   void onButton(int button, int action, int mods, double x, double y)
   {
      MouseEvent e(MouseEvent::Type::Button);
      e.button = button;
      e.action = action;
      e.mods = mods;
      e.x = x;
      e.y = y;
      m_events.push_back(e);
   }

   void onMove(double x, double y)
   {
      MouseEvent e(MouseEvent::Type::Move);
      e.x = x;
      e.y = y;
      m_events.push_back(e);

   }

   void onEnter(bool entered, double x, double y)
   {
      MouseEvent e(MouseEvent::Type::Enter);
      e.entered = entered;
      e.x = x;
      e.y = y;
      m_events.push_back(e);
   }

   void onScroll(double offX, double offY, int mods, double posX, double posY)
   {
      MouseEvent e(MouseEvent::Type::Scroll);
      e.scrollX = offX;
      e.scrollY = offY;
      e.mods = mods;
      e.x = posX;
      e.y = posY;
      m_events.push_back(e);
   }

   void buildVPStack(std::vector<UIElement *> &stack, UIElement *element)
   {
      stack.push_back(element);
      if(auto layout = element->getLayout())
      {
         for(auto child : layout->getChildren())
            buildVPStack(stack, child);
      }
   }

   //determine if an element is receiving focus and give it
   void resolveFocus(UIElement *ui, MouseEvent &e)
   {
      if(!m_focusedElement)
      {
         if(e.type == MouseEvent::Type::Button &&
            e.action == Input::Press)
         {
            m_focusedElement = ui;
            m_focusedButton = e.button;
            ui->giveMouseFocus(parent);
         }
      }
      else
      {
         if(e.type == MouseEvent::Type::Button &&
            e.action == Input::Release)
         {
            m_focusedElement = nullptr;
            ui->removeMouseFocus();
         }
      }
         
   }

   void resetMouseFocus(){m_focusedElement = nullptr;}

   void performMouseEvents(UIElement &rootElement)
   {
      std::vector<UIElement *> stack;
      buildVPStack(stack, &rootElement);

      for(auto &e : m_events)
      {
         //if an element has focus, all mouse data goes to that
         if(m_focusedElement)
         {
            m_focusedElement->onMouseEvent(e);
            resolveFocus(m_focusedElement, e);
            continue;
         }

         int i = 0;
         for(auto iter = stack.rbegin(); iter < stack.rend(); ++iter)
         {
            auto element = *iter;
            ++i;

            if(element->getBounds().outerClipped.contains(Float2((float)e.x, (float)e.y)) || 
               e.type == MouseEvent::Type::Enter)
            {
               if(element->onMouseEvent(e))
               {
                  resolveFocus(element, e);
                  break;
               }
               //if the event has mods and the element rejects it, 
               //try sending it unmodded
               else if(e.mods && 
                  (e.type == MouseEvent::Type::Button || 
                     e.type == MouseEvent::Type::Scroll))
               {
                  MouseEvent unModded(e);
                  unModded.mods = 0;
                  if(element->onMouseEvent(e))
                  {
                     resolveFocus(element, e);
                     break;
                  }
               }
            }
         }
      }


      m_events.clear();
   }
};

MouseHandler::MouseHandler(GLWindow *win)
   :pImpl(new Impl(win, this)){}
MouseHandler::~MouseHandler(){}
void MouseHandler::performMouseEvents(UIElement &rootElement){pImpl->performMouseEvents(rootElement);}
void MouseHandler::resetMouseFocus(){pImpl->resetMouseFocus();}