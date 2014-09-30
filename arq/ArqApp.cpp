#include <GLFW\glfw3.h>
#include "ArqApp.h"

#include "engine\CoreUI.h"
#include "engine\UIDecorator.h"
#include "engine\IOCContainer.h"
#include "engine\StringTable.h"
#include "arq-game\ArqGame.h"
#include "logbadger\LogBadger.h"
#include "engine\InputDefinitions.h"

class ArqApp : public Application
{
   std::unique_ptr<Tool> m_arqGame, m_logBadger;
public:
   ~ArqApp(){}

   Int2 getDefaultWindowSize()
   {
      return Int2(1600, 900);
      //return Int2(1920, 1080);
   }

   std::string getWindowTitle()
   {
      return "Legend of Shitty Game Clones: Arqarino of Timerino";
   }

   //GLFWmonitor *getWindowMonitor()
   //{
   //   return glfwGetPrimaryMonitor();
   //}

   void buildSplashBG()
   {
      auto st = IOC.resolve<StringTable>();
      auto splash = getUIRoot().intern(
         buildBackgroundedElement(
         CoreUI::buildBasicElement(Rectf(), CoreUI::Layouts::buildFreeLayout())));
      splash->anchorToParent();
      splash->setOption(UIOption::BackgroundType, UIBackgroundType::Image);
      splash->setOption(UIOption::BackgroundImage, st->get("assets/img/splash.png"));

      getUIRoot().pushListChild(splash);

   }
   
   void onAppStart()
   {
      buildSplashBG();

      m_arqGame = createArqGame();
      m_arqGame->open(&getUIRoot());
      m_arqGame->setMaximized(true);
      giveFocus(m_arqGame->getDlgElement());

      m_logBadger = createLogBadger();
      m_logBadger->open(&getUIRoot());

      getUIRoot().arrange(); 
      getUIRoot().registerKeyboardKey(Input::KeyEscape, Input::Release, 0, [&](KeyEvent e){shutdown();});


      
   }
};

std::unique_ptr<Application> buildArqApp()
{
   return std::unique_ptr<Application>(new ArqApp());
}