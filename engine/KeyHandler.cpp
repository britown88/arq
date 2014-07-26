#include "KeyHandler.h"
#include "GLWindow.h"
#include "UIElement.h"
#include "InputDefinitions.h"

#include <vector>

class KeyHandler::Impl
{
   std::vector<MouseEvent> m_events;

   UIElement *m_focusedElement;
   int m_focusedButton;

   KeyHandler *parent;
   GLWindow *m_window;

public:
   Impl(GLWindow *win, KeyHandler *parent):m_window(win), m_focusedElement(nullptr), parent(parent)
   {
   }
   ~Impl(){}

   

   void resetKeyFocus(){m_focusedElement = nullptr;}

   void performKeyEvents(UIElement &rootElement)
   {
      
   }
};

KeyHandler::KeyHandler(GLWindow *win)
   :pImpl(new Impl(win, this)){}
KeyHandler::~KeyHandler(){}
void KeyHandler::performKeyEvents(UIElement &rootElement){pImpl->performKeyEvents(rootElement);}
void KeyHandler::resetKeyFocus(){pImpl->resetKeyFocus();}