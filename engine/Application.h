#pragma once

#include "GLWindow.h"
#include <memory>
#include <stdlib.h>
#include <time.h>
#include "Renderer.h"
#include "UIElement.h"
#include "MouseHandler.h"
#include "KeyHandler.h"
#include "ControllerHandler.h"

class Application
{
   bool m_appRunning;
   double m_frameRate, m_dt, m_lastUpdated;   
   std::unique_ptr<GLWindow> m_window;
   std::unique_ptr<Renderer> m_renderer;
   std::unique_ptr<UIElement> m_rootUIElement;
   std::unique_ptr<MouseHandler> m_mouseHandler;
   std::unique_ptr<KeyHandler> m_keyHandler;
   std::unique_ptr<ControllerHandler> m_controllerHandler;

   std::vector<std::unique_ptr<UIElement>> m_deletedUIItems;

   UIElement *m_focusedElement;
   UIElement *_focusedElement();

   
   void terminate();
   bool isRunning();
   void step();

public:
   Application();
   virtual ~Application();

   void start();
   void shutdown();

   Int2 windowSize();


   double getTime() const;
   double frameTime() const;
   double frameRate() const;
   double dt() const;

   int rand(int lower, int upper);

   Float2 getMousePosition();

   void deleteUIElement(std::unique_ptr<UIElement> e);

   void giveFocus(UIElement *e);
   void removeFocus(UIElement *e);

   int getKeyState(int key);
   int getJoystickButtonState(int id, int button);
   int getJoystickAxisState(int id, int axis);
   

protected:
   Renderer &getRenderer();
   UIElement &getUIRoot();

   virtual double getFrameRate() const;
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
