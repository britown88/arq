#include "InputManager.h"
#include "engine\Component.h"
#include "ArqComponents.h"
#include "GameData.h"
#include "ActorManager.h"
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

      registerKeyboard();
   }

   ~InputManagerImpl()
   {
      unregisterKeyboard();
   }

   void registerKeyboard()
   {
      auto leftPress = [=](KeyEvent ce){onLeftPress(0);};
      auto leftRelease = [=](KeyEvent ce){onLeftRelease(0);};
      auto rightPress = [=](KeyEvent ce){onRightPress(0);};
      auto rightRelease = [=](KeyEvent ce){onRightRelease(0);};
      auto upPress = [=](KeyEvent ce){onUpPress(0);};
      auto upRelease = [=](KeyEvent ce){onUpRelease(0);};
      auto downPress = [=](KeyEvent ce){onDownPress(0);};
      auto downRelease = [=](KeyEvent ce){onDownRelease(0);};

      //auto APress = [=](KeyEvent ce){onAPress(0);};
      //auto ARelease = [=](KeyEvent ce){onARelease(0);};

      m_element->registerKeyboardKey(Input::KeyLeft, Input::Press, 0, leftPress);
      m_element->registerKeyboardKey(Input::KeyLeft, Input::Release, 0, leftRelease);
      m_element->registerKeyboardKey(Input::KeyRight, Input::Press, 0, rightPress);
      m_element->registerKeyboardKey(Input::KeyRight, Input::Release, 0, rightRelease);
      m_element->registerKeyboardKey(Input::KeyDown, Input::Press, 0, downPress);
      m_element->registerKeyboardKey(Input::KeyDown, Input::Release, 0, downRelease);
      m_element->registerKeyboardKey(Input::KeyUp, Input::Press, 0, upPress);
      m_element->registerKeyboardKey(Input::KeyUp, Input::Release, 0, upRelease);
      //m_element->registerKeyboardKey(Input::KeySpace, Input::Press, 0, APress);
      //m_element->registerKeyboardKey(Input::KeySpace, Input::Release, 0, ARelease);

   }

   void unregisterKeyboard()
   {
      m_element->unregisterKeyboardKey(Input::KeyLeft, Input::Press, 0);
      m_element->unregisterKeyboardKey(Input::KeyLeft, Input::Release, 0);
      m_element->unregisterKeyboardKey(Input::KeyRight, Input::Press, 0);
      m_element->unregisterKeyboardKey(Input::KeyRight, Input::Release, 0);
      //m_element->unregisterKeyboardKey(Input::KeySpace, Input::Press, 0);
      //m_element->unregisterKeyboardKey(Input::KeySpace, Input::Release, 0);
      m_element->unregisterKeyboardKey(Input::KeyUp, Input::Press, 0);
      m_element->unregisterKeyboardKey(Input::KeyUp, Input::Release, 0);
      m_element->unregisterKeyboardKey(Input::KeyDown, Input::Press, 0);
      m_element->unregisterKeyboardKey(Input::KeyDown, Input::Release, 0);
   }

   void onLeftPress(int player){m_system->getManager<ActorManager>()->moveLeft(player);}   
   void onLeftRelease(int player){m_system->getManager<ActorManager>()->stopLeft(player);}
   void onRightPress(int player){m_system->getManager<ActorManager>()->moveRight(player);}
   void onRightRelease(int player){m_system->getManager<ActorManager>()->stopRight(player);}
   void onUpPress(int player){m_system->getManager<ActorManager>()->moveUp(player);}
   void onUpRelease(int player){m_system->getManager<ActorManager>()->stopUp(player);}
   void onDownPress(int player){m_system->getManager<ActorManager>()->moveDown(player);}
   void onDownRelease(int player){m_system->getManager<ActorManager>()->stopDown(player);}

   static void registerComponentCallbacks(Manager<InputManagerImpl, InputManager> &m)
   {
   }

};

std::unique_ptr<InputManager> buildInputManager(UIElement *element)
{
   return std::unique_ptr<InputManager>(new InputManagerImpl(element));
}