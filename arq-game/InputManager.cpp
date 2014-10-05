#include "ArqManagers.h"
#include "engine\Component.h"
#include "ArqComponents.h"
#include "GameData.h"
#include "ActorManager.h"
#include "engine\InputDefinitions.h"
#include "engine\InputDefinitions360.h"
#include "engine\KeyEvent.h"
#include "engine\IOCContainer.h"
#include "engine\Application.h"
#include "GameData.h"

REGISTER_COMPONENT(PlayerControlledComponent);

class InputManagerImpl : public Manager<InputManagerImpl, InputManager>
{
   CoreUI::WorldUIElement  *m_element;
   Application &app;
public:
   InputManagerImpl(CoreUI::WorldUIElement *element):m_element(element), app(*IOC.resolve<Application>())
   {
      registerKeyboard();
   }

   ~InputManagerImpl()
   {
      unregisterKeyboard();
   }

   void registerKeyboard()
   {
      auto leftPress = [=](KeyEvent ce){onLeftPress();};
      auto leftRelease = [=](KeyEvent ce){onLeftRelease();};
      auto rightPress = [=](KeyEvent ce){onRightPress();};
      auto rightRelease = [=](KeyEvent ce){onRightRelease();};
      auto upPress = [=](KeyEvent ce){onUpPress();};
      auto upRelease = [=](KeyEvent ce){onUpRelease();};
      auto downPress = [=](KeyEvent ce){onDownPress();};
      auto downRelease = [=](KeyEvent ce){onDownRelease();};

      //auto APress = [=](KeyEvent ce){onAPress(0);};
      //auto ARelease = [=](KeyEvent ce){onARelease(0);};

      m_element->registerKeyboardKey(Input::KeyA, Input::Press, 0, leftPress);
      m_element->registerKeyboardKey(Input::KeyA, Input::Release, 0, leftRelease);
      m_element->registerKeyboardKey(Input::KeyD, Input::Press, 0, rightPress);
      m_element->registerKeyboardKey(Input::KeyD, Input::Release, 0, rightRelease);
      m_element->registerKeyboardKey(Input::KeyS, Input::Press, 0, downPress);
      m_element->registerKeyboardKey(Input::KeyS, Input::Release, 0, downRelease);
      m_element->registerKeyboardKey(Input::KeyW, Input::Press, 0, upPress);
      m_element->registerKeyboardKey(Input::KeyW, Input::Release, 0, upRelease);
      //m_element->registerKeyboardKey(Input::KeySpace, Input::Press, 0, APress);
      //m_element->registerKeyboardKey(Input::KeySpace, Input::Release, 0, ARelease);

      m_element->registerMouseButton(Input::MouseLeft, Input::Press, 0, [=](MouseEvent e){onLeftMousePress((float)e.x, (float)e.y);});
      m_element->registerMouseButton(Input::MouseLeft, Input::Release, 0, [=](MouseEvent e){onLeftMouseRelease((float)e.x, (float)e.y);});
      m_element->registerMouseButton(Input::MouseRight, Input::Press, 0, [=](MouseEvent e){onRightMousePress((float)e.x, (float)e.y);});
      m_element->registerMouseButton(Input::MouseRight, Input::Release, 0, [=](MouseEvent e){onRightMouseRelease((float)e.x, (float)e.y);});

   }

   void unregisterKeyboard()
   {
      m_element->unregisterKeyboardKey(Input::KeyA, Input::Press, 0);
      m_element->unregisterKeyboardKey(Input::KeyA, Input::Release, 0);
      m_element->unregisterKeyboardKey(Input::KeyD, Input::Press, 0);
      m_element->unregisterKeyboardKey(Input::KeyD, Input::Release, 0);
      //m_element->unregisterKeyboardKey(Input::KeySpace, Input::Press, 0);
      //m_element->unregisterKeyboardKey(Input::KeySpace, Input::Release, 0);
      m_element->unregisterKeyboardKey(Input::KeyW, Input::Press, 0);
      m_element->unregisterKeyboardKey(Input::KeyW, Input::Release, 0);
      m_element->unregisterKeyboardKey(Input::KeyS, Input::Press, 0);
      m_element->unregisterKeyboardKey(Input::KeyS, Input::Release, 0);

      m_element->unregisterMouseButton(Input::MouseLeft, Input::Press, 0);
      m_element->unregisterMouseButton(Input::MouseLeft, Input::Release, 0);
      m_element->unregisterMouseButton(Input::MouseRight, Input::Press, 0);
      m_element->unregisterMouseButton(Input::MouseRight, Input::Release, 0);
   }

   void sendInput(std::function<void(Entity*)> func)
   {
      for(auto &comp : m_system->getComponentVector<PlayerControlledComponent>())
         func(comp.parent);
   }

   void onLeftPress(){sendInput([&](Entity *e){m_system->getManager<ActorManager>()->moveLeft(e);});}
   void onLeftRelease(){sendInput([&](Entity *e){m_system->getManager<ActorManager>()->stopLeft(e);});}
   void onRightPress(){sendInput([&](Entity *e){m_system->getManager<ActorManager>()->moveRight(e);});}
   void onRightRelease(){sendInput([&](Entity *e){m_system->getManager<ActorManager>()->stopRight(e);});}
   void onUpPress(){sendInput([&](Entity *e){m_system->getManager<ActorManager>()->moveUp(e);});}
   void onUpRelease(){sendInput([&](Entity *e){m_system->getManager<ActorManager>()->stopUp(e);});}
   void onDownPress(){sendInput([&](Entity *e){m_system->getManager<ActorManager>()->moveDown(e);});}
   void onDownRelease(){sendInput([&](Entity *e){m_system->getManager<ActorManager>()->stopDown(e);});}

   void sendActionInput(ActionType type, bool execute, Float2 target)
   {
      for(auto &comp : m_system->getComponentVector<PlayerControlledComponent>())
      {
         if(execute)
            m_system->getManager<ActorManager>()->executeAction(comp.parent, type, m_element->vp2world(target));
         else
            m_system->getManager<ActorManager>()->endAction(comp.parent, type, m_element->vp2world(target));
      } 
   }

   void onLeftMousePress(float x, float y){sendActionInput(ActionType::MainHand, true, Float2(x, y));}
   void onLeftMouseRelease(float x, float y){sendActionInput(ActionType::MainHand, false, Float2(x, y));}
   void onRightMousePress(float x, float y){sendActionInput(ActionType::OffHand, true, Float2(x, y));}
   void onRightMouseRelease(float x, float y){sendActionInput(ActionType::OffHand, false, Float2(x, y));}

   static void registerComponentCallbacks(Manager<InputManagerImpl, InputManager> &m)
   {
   }

};

std::unique_ptr<InputManager> buildInputManager(CoreUI::WorldUIElement *element)
{
   return std::unique_ptr<InputManager>(new InputManagerImpl(element));
}