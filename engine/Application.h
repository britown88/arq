#pragma once

#include "GLWindow.h"
#include <memory>
#include <stdlib.h>
#include <time.h>
#include "Renderer.h"
#include "UIElement.h"
#include "MouseHandler.h"

class Application
{
   bool m_appRunning;
   double m_frameRate, m_dt, m_lastUpdated;   
   std::unique_ptr<GLWindow> m_window;
   std::unique_ptr<Renderer> m_renderer;
   std::unique_ptr<UIElement> m_rootUIElement;
   std::unique_ptr<MouseHandler> m_mouseHandler;   

   std::vector<std::unique_ptr<UIElement>> m_deletedUIItems;
public:
   void start();
   void terminate();
   bool isRunning();
   void step();

   Int2 windowSize();

   double getTime();
   double frameTime();
   double dt();

   int rand(int lower, int upper);

   Float2 getMousePosition();

   void deleteUIElement(std::unique_ptr<UIElement> e);

protected:
   Renderer &getRenderer();
   UIElement &getUIRoot();

   virtual double getFrameRate();
   virtual Int2 getDefaultWindowSize();
   virtual std::string getWindowTitle();
   virtual GLFWmonitor *getWindowMonitor();

   virtual void onAppStart(){}
   virtual void onStep(){}
   virtual void onTerminate(){}
};

template<typename T>
Application &registerApplication(std::unique_ptr<T> app)
{
   IOC.add<Application>(std::move(app));
   return *IOC.resolve<Application>();
}
