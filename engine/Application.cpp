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

#include <chrono>
#include <thread>

IOCContainer IOC;
std::vector<ComponentTypeVTable*> vTables;

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
   IOC.add(std::unique_ptr<TextureManager>(new TextureManager()));
   IOC.add(std::unique_ptr<FontEngine>(new FontEngine()));

   m_window.reset(new GLWindow(winSize, winTitle, winMonitor));
   m_renderer.reset(new Renderer(m_window.get()));
   m_rootUIElement = CoreUI::buildRootUIElement();
   m_mouseHandler.reset(new MouseHandler(m_window.get()));

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

Renderer &Application::getRenderer(){return *m_renderer.get();}
UIElement &Application::getUIRoot(){return *m_rootUIElement.get();}

Int2 Application::windowSize()
{
   return m_window->getSize();
}

double Application::getTime()
{
   return glfwGetTime() * 1000;
}

double Application::dt()
{
   return m_dt;
}

double Application::frameTime()
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

void Application::step()
{
   //dt
   auto time = getTime();
   double deltaTime = time - m_lastUpdated;
   m_lastUpdated = time;
   m_dt = deltaTime / frameTime();

   //update
   if(m_dt > 0.0f)
   {
      onStep();
      m_rootUIElement->update();
   }      

   //input
   m_window->pollEvents();
   m_mouseHandler->performMouseEvents(*m_rootUIElement.get());

   //render
   m_rootUIElement->draw(*m_renderer.get());

   for(auto &e : m_deletedUIItems)
      e.reset();
   m_deletedUIItems.clear();

   if(m_window->shouldClose())
      m_appRunning = false;  
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

double Application::getFrameRate()
{
   return 60.0;
}


