#include <GLFW/glfw3.h>
#include "Application.h"
#include "IOCContainer.h"
#include "GnUtilities.h"
#include "StringTable.h"
#include "DrawBuffer.h"
#include "AssetManager.h"
#include "Renderer.h"
#include "CoreComponents.h"
#include "CoreUI.h"
#include "TextureManager.h"
#include "Text.h"
#include "DataManager.h"
#include "SpriteManager.h"
#include "Logs.h"

#include <chrono>
#include <thread>

IOCContainer IOC;
std::vector<ComponentTypeVTable*> vTables;

Application::Application():m_focusedElement(nullptr){}
Application::~Application(){}

void Application::start()
{
   srand (time(NULL));
   m_lastUpdated = 0.0;
   m_dt = 1.0;
   m_frameRate = getFrameRate();

   if(!glfwInit())
      throw ArqException("GLFW Failed to initialize");

   auto winSize = getDefaultWindowSize();
   auto winTitle = getWindowTitle();
   auto winMonitor = getWindowMonitor();

   IOC.add(std::unique_ptr<StringTable>(new StringTable()));  
   IOC.add(std::unique_ptr<AssetManager>(new AssetManager()));
   IOC.add(std::unique_ptr<DataManager>(new DataManager()));
   IOC.add(std::unique_ptr<SpriteManager>(new SpriteManager()));
   IOC.add(std::unique_ptr<TextureManager>(new TextureManager()));
   IOC.add(std::unique_ptr<FontEngine>(new FontEngine()));
   IOC.add(std::unique_ptr<LogManager>(new LogManager()));

   m_window.reset(new GLWindow(winSize, winTitle, winMonitor));
   m_renderer.reset(new Renderer(m_window.get()));
   m_rootUIElement = CoreUI::buildRootUIElement();
   m_mouseHandler.reset(new MouseHandler(m_window.get()));
   m_keyHandler.reset(new KeyHandler(m_window.get()));
   m_controllerHandler.reset(new ControllerHandler(m_window.get()));

   onAppStart();

   m_appRunning = true;

   while(true)
   {
      step();

      if(!isRunning())           
         break;            
   }  

   terminate(); 
}

void Application::shutdown()
{
   m_appRunning = false;  
}

Renderer &Application::getRenderer(){return *m_renderer.get();}
UIElement &Application::getUIRoot(){return *m_rootUIElement.get();}

Int2 Application::windowSize()
{
   return m_window->getSize();
}

double Application::getTime() const
{
   return glfwGetTime() * 1000;
}

double Application::dt() const
{
   return m_dt;
}

double Application::frameRate() const
{
   return getFrameRate();
}

double Application::frameTime() const
{
   return 1000.0 / m_frameRate;
}

Float2 Application::getMousePosition()
{
   return m_window->getMousePosition();
}

int Application::rand(int lower, int upper)
{
   return (::rand() % (upper - lower)) + lower;
}

void Application::deleteUIElement(std::unique_ptr<UIElement> e)
{
   m_deletedUIItems.push_back(std::move(e));
}

void Application::giveFocus(UIElement *e)
{
   m_focusedElement = e;
}

void Application::removeFocus(UIElement *e)
{
   if(e)
   {
      if(e == m_focusedElement)
         m_focusedElement = nullptr;
   }
   else
   {
      m_focusedElement = nullptr;
   }
      
}
UIElement *Application::_focusedElement()
{
   if(m_focusedElement)
      return m_focusedElement;
   else
      return m_rootUIElement.get();
}

int Application::getKeyState(int key){return m_window->getKeyState(key);}
int Application::getJoystickButtonState(int id, int button){return m_window->getJoystickButtonState(id, button);}
int Application::getJoystickAxisState(int id, int axis){return m_window->getJoystickAxisState(id, axis);}

void Application::step()
{
   //dt
   auto time = getTime();
   double deltaTime = time - m_lastUpdated;
   
   m_dt = deltaTime / frameTime();

   if(m_dt > 2.0)
      m_dt = 2.0;

   //m_dt = 0.2f;
   //Logs::d("App") << "DT: " << deltaTime;

   //update
   if(m_dt >= 1.0f)
   {
      m_lastUpdated = time;

      onStep();
      m_rootUIElement->update();

      //input
      m_window->pollEvents();
      m_mouseHandler->performMouseEvents(*m_rootUIElement.get());

      if(auto fe = _focusedElement())
      {
         m_keyHandler->performKeyEvents(*fe);
         m_controllerHandler->performControllerEvents(*fe);
      }

         //render
      m_rootUIElement->draw(*m_renderer.get());

      for(auto &e : m_deletedUIItems)
         e.reset();
      m_deletedUIItems.clear();

      if(m_window->shouldClose())
         m_appRunning = false;  
   }
   else
      std::this_thread::yield();


}

void Application::terminate()
{
   m_renderer->terminate();
   onTerminate();
   IOC.clear();
}

bool Application::isRunning()
{
   return m_appRunning;
}

Int2 Application::getDefaultWindowSize()
{
   return Int2(800, 600);
}

std::string Application::getWindowTitle()
{
   return "Window Title";
}

GLFWmonitor *Application::getWindowMonitor()
{
   return nullptr;
}

double Application::getFrameRate() const
{
   return 60.0;
}


