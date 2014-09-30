#include "InputManager.h"
#include "engine\Component.h"
#include "ArqComponents.h"
#include "GameData.h"
#include "CharacterManager.h"
#include "engine\InputDefinitions.h";
#include "engine\InputDefinitions360.h"
#include "engine\KeyEvent.h"
#include "engine\IOCContainer.h"
#include "engine\Application.h"

struct InputPlayer
{
   int controllerId;
   bool attached;

   InputPlayer():attached(false), controllerId(0){}
};

class InputManagerImpl : public Manager<InputManagerImpl, InputManager>
{
   UIElement *m_element;
   std::vector<InputPlayer> m_connectedPlayers;
   Application &app;
public:
   InputManagerImpl(UIElement *element):m_element(element), app(*IOC.resolve<Application>())
   {
      m_connectedPlayers.resize(GameData::PlayerCount, InputPlayer());

      m_element->registerControllerPresence(true, [=](ControllerEvent ce){onControllerConnect(ce.id);});
      m_element->registerControllerPresence(false, [=](ControllerEvent ce){onControllerDisconnect(ce.id);});

      registerKeyboard();
   }

   ~InputManagerImpl()
   {
      m_element->unregisterControllerPresence(true);
      m_element->unregisterControllerPresence(false);

      int i = 0;
      for(auto &c : m_connectedPlayers)
      {
         if(c.attached)
            unregisterPlayer(c.controllerId, i);         
         ++i;
      }

      unregisterKeyboard();
   }

   void registerKeyboard()
   {
      auto leftPress = [=](KeyEvent ce){onLeftPress(0);};
      auto leftRelease = [=](KeyEvent ce)
      {
         if(app.getKeyState(Input::KeyRight) != Input::Press)
            onLeftRelease(0);
      };
      auto rightPress = [=](KeyEvent ce){onRightPress(0);};
      auto rightRelease = [=](KeyEvent ce)
      {
         if(app.getKeyState(Input::KeyLeft) != Input::Press)
            onRightRelease(0);
      };

      auto APress = [=](KeyEvent ce){onAPress(0);};
      auto ARelease = [=](KeyEvent ce){onARelease(0);};

      m_element->registerKeyboardKey(Input::KeyLeft, Input::Press, 0, leftPress);
      m_element->registerKeyboardKey(Input::KeyLeft, Input::Release, 0, leftRelease);
      m_element->registerKeyboardKey(Input::KeyRight, Input::Press, 0, rightPress);
      m_element->registerKeyboardKey(Input::KeyRight, Input::Release, 0, rightRelease);
      m_element->registerKeyboardKey(Input::KeySpace, Input::Press, 0, APress);
      m_element->registerKeyboardKey(Input::KeySpace, Input::Release, 0, ARelease);

   }

   void unregisterKeyboard()
   {
      m_element->unregisterKeyboardKey(Input::KeyLeft, Input::Press, 0);
      m_element->unregisterKeyboardKey(Input::KeyLeft, Input::Release, 0);
      m_element->unregisterKeyboardKey(Input::KeyRight, Input::Press, 0);
      m_element->unregisterKeyboardKey(Input::KeyRight, Input::Release, 0);
      m_element->unregisterKeyboardKey(Input::KeySpace, Input::Press, 0);
      m_element->unregisterKeyboardKey(Input::KeySpace, Input::Release, 0);
   }

   void registerPlayer(int controllerId, int playerNumber)
   {
      auto leftPress = [=](ControllerEvent ce){onLeftPress(playerNumber);};
      auto leftRelease = [=](ControllerEvent ce)
      {
         if(app.getJoystickAxisState(ce.id, Input::XBOCT360::LeftStickRight) != Input::Press && 
            app.getJoystickButtonState(ce.id, Input::XBOCT360::DPadRight) != Input::Press)
            onLeftRelease(playerNumber);
      };
      auto rightPress = [=](ControllerEvent ce){onRightPress(playerNumber);};
      auto rightRelease = [=](ControllerEvent ce)
      {
         if(app.getJoystickAxisState(ce.id, Input::XBOCT360::LeftStickLeft) != Input::Press && 
            app.getJoystickButtonState(ce.id, Input::XBOCT360::DPadLeft) != Input::Press)
            onRightRelease(playerNumber);
      };

      auto APress = [=](ControllerEvent ce){onAPress(playerNumber);};
      auto ARelease = [=](ControllerEvent ce){onARelease(playerNumber);};

      m_element->registerControllerButton(controllerId, Input::XBOCT360::DPadLeft, Input::Press, leftPress);
      m_element->registerControllerAxis(controllerId, Input::XBOCT360::LeftStickLeft, Input::Press, leftPress);

      m_element->registerControllerButton(controllerId, Input::XBOCT360::DPadLeft, Input::Release, leftRelease);
      m_element->registerControllerAxis(controllerId, Input::XBOCT360::LeftStickLeft, Input::Release, leftRelease);

      m_element->registerControllerButton(controllerId, Input::XBOCT360::DPadRight, Input::Press, rightPress);
      m_element->registerControllerAxis(controllerId, Input::XBOCT360::LeftStickRight, Input::Press, rightPress);

      m_element->registerControllerButton(controllerId, Input::XBOCT360::DPadRight, Input::Release, rightRelease);
      m_element->registerControllerAxis(controllerId, Input::XBOCT360::LeftStickRight, Input::Release, rightRelease);

      m_element->registerControllerButton(controllerId, Input::XBOCT360::A, Input::Press, APress);
      m_element->registerControllerButton(controllerId, Input::XBOCT360::A, Input::Release, ARelease);

   }

   void unregisterPlayer(int controllerId, int playerNumber)
   {

      m_element->unregisterControllerButton(controllerId, Input::XBOCT360::DPadLeft, Input::Press);
      m_element->unregisterControllerAxis(controllerId, Input::XBOCT360::LeftStickLeft, Input::Press);

      m_element->unregisterControllerButton(controllerId, Input::XBOCT360::DPadLeft, Input::Release);
      m_element->unregisterControllerAxis(controllerId, Input::XBOCT360::LeftStickLeft, Input::Release);

      m_element->unregisterControllerButton(controllerId, Input::XBOCT360::DPadRight, Input::Press);
      m_element->unregisterControllerAxis(controllerId, Input::XBOCT360::LeftStickRight, Input::Press);

      m_element->unregisterControllerButton(controllerId, Input::XBOCT360::DPadRight, Input::Release);
      m_element->unregisterControllerAxis(controllerId, Input::XBOCT360::LeftStickRight, Input::Release);

      m_element->unregisterControllerButton(controllerId, Input::XBOCT360::A, Input::Press);
      m_element->unregisterControllerButton(controllerId, Input::XBOCT360::A, Input::Release);
   }

   void onLeftPress(int player){m_system->getManager<CharacterManager>()->moveLeft(player);}
   void onLeftRelease(int player){m_system->getManager<CharacterManager>()->stop(player);}
   void onRightPress(int player){m_system->getManager<CharacterManager>()->moveRight(player);}
   void onRightRelease(int player){m_system->getManager<CharacterManager>()->stop(player);}
   void onAPress(int player){m_system->getManager<CharacterManager>()->jump(player);}
   void onARelease(int player){m_system->getManager<CharacterManager>()->endJump(player);}

   void onControllerConnect(int id)
   {
      int i = 0;
      for(auto &c : m_connectedPlayers)
      {
         if(!c.attached)
         {
            c.attached = true;
            c.controllerId = id;
            registerPlayer(id, i);
            return;
         }

         ++i;
      }
   }

   void onControllerDisconnect(int id)
   {
      int i = 0;
      for(auto &c : m_connectedPlayers)
      {
         if(c.controllerId == id)
         {
            c.attached = false;
            unregisterPlayer(id, i);
            return;
         }

         ++i;
      }
   }

   static void registerComponentCallbacks(Manager<InputManagerImpl, InputManager> &m)
   {
   }

};

std::unique_ptr<InputManager> buildInputManager(UIElement *element)
{
   return std::unique_ptr<InputManager>(new InputManagerImpl(element));
}