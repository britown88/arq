#include "KeyHandler.h"
#include "GLWindow.h"
#include "UIElement.h"
#include "InputDefinitions.h"
#include "KeyEvent.h"

#include <vector>

class KeyHandler::Impl
{
   std::vector<KeyEvent> m_events;

   int m_focusedButton;

   KeyHandler *parent;
   GLWindow *m_window;

public:
   Impl(GLWindow *win, KeyHandler *parent):m_window(win), parent(parent)
   {
      win->setKeyCallback([&](int key, int action, int mods){onKey(key, action, mods);});
      win->setCharCallback([&](unsigned int character){onChar(character);});
   }
   ~Impl(){}

   void onKey(int key, int action, int mods)
   {
      KeyEvent ke(KeyEvent::Type::Key);
      ke.key = key;
      ke.action = action;
      ke.mods = mods;
      m_events.push_back(ke);
   }

   void onChar(unsigned int character)
   {
      KeyEvent ke(KeyEvent::Type::Char);
      ke.character = character;
      m_events.push_back(ke);
   }

   void performKeyEvents(UIElement &focusedElement)
   {
      for(auto &event : m_events)
      {
         auto e = &focusedElement;
         while(e)
         {
            if(e->onKeyEvent(event))
               break;
            e = e->getParent();
         }
      }

      m_events.clear();
      
   }
};

KeyHandler::KeyHandler(GLWindow *win)
   :pImpl(new Impl(win, this)){}
KeyHandler::~KeyHandler(){}
void KeyHandler::performKeyEvents(UIElement &focusedElement){pImpl->performKeyEvents(focusedElement);}