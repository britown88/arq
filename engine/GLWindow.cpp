#include "GL/glew.h"
#include <GLFW/glfw3.h>
#include "GLWindow.h"

#include "GnUtilities.h"
#include <boost\optional.hpp>

#include "Logs.h"

static const std::string InputTag = "Input";
static int KeyStates[256];

#define GLFW_JOYSTICK_COUNT GLFW_JOYSTICK_LAST + 1

GLWindow::MouseButtonCallback _CBbtn;
GLWindow::MousePositionCallback _CBpos;
GLWindow::MouseEnterCallback _CBenter;
GLWindow::MouseScrollCallback _CBscrl;
GLWindow::KeyCallback _CBkey;
GLWindow::CharCallback _CBchar;

static const float JoystickThreshold = 0.5f;

void _mButton(GLFWwindow *win, int btn, int act, int mod)
{
   double x, y;
   glfwGetCursorPos(win, &x, &y);

   if(act == GLFW_RELEASE)
      mod = 0;

   _CBbtn(btn, act, mod, x, y);
}
void _mPosition(GLFWwindow *win, double x, double y){_CBpos(x, y);}
void _mEnter(GLFWwindow *win, int enter)
{
   double x, y;
   glfwGetCursorPos(win, &x, &y);
   _CBenter(enter != 0, x, y);
}
void _mScroll(GLFWwindow *win, double x, double y)
{
   double px, py;
   glfwGetCursorPos(win, &px, &py);

   //set mods ourself for attaching mod to a scroll
   int mods = 0;
   if(glfwGetKey(win, GLFW_KEY_LEFT_ALT) == GLFW_PRESS || 
      glfwGetKey(win, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS)
      mods |= GLFW_MOD_ALT;

   if(glfwGetKey(win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || 
      glfwGetKey(win, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)
      mods |= GLFW_MOD_SHIFT;

   if(glfwGetKey(win, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || 
      glfwGetKey(win, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS)
      mods |= GLFW_MOD_CONTROL;

   _CBscrl(x, y, mods, px, py);
}
void _key(GLFWwindow *win, int key, int scan, int act, int mod)
{
   KeyStates[key] = act;
   _CBkey(key, act, mod);
}
void _char(GLFWwindow *win, unsigned int c){_CBchar(c);}

struct ControllerState
{
   bool attached;

   std::vector<bool> buttons;
   std::vector<bool> axes;

   char *name;

   ControllerState():attached(false){}
};

class GLWindow::Impl
{
   GLFWwindow* m_window, *m_threadWin;
   Int2 m_windowSize;
   bool m_fullscreen;

   boost::optional<GLWindow::ControllerPresenceCallback> m_cbConPres;
   boost::optional<GLWindow::ControllerButtonCallback> m_cbConButton;
   boost::optional<GLWindow::ControllerAxisCallback> m_cbConAxis;

   ControllerState m_controllers[GLFW_JOYSTICK_COUNT];

   int m_controllerPresenceCheck;

public:
   Impl(Int2 winSize, std::string windowName, GLFWmonitor *monitor = nullptr):m_controllerPresenceCheck(0)
   {
      memset(&KeyStates, 0, 256 * sizeof(int));
      glfwWindowHint( GLFW_VISIBLE, GL_FALSE );
      m_threadWin = glfwCreateWindow( 1, 1, "Thread Window", NULL, NULL );

      glfwWindowHint( GLFW_VISIBLE, GL_TRUE );
      if(!monitor) glfwWindowHint(GLFW_RESIZABLE, 0);
      m_window = glfwCreateWindow(winSize.x, winSize.y, windowName.c_str(), monitor, m_threadWin);
      //glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

      if(!m_window)
         throw ArqException("Window failed to create");

      glfwGetFramebufferSize(m_window, &m_windowSize.x, &m_windowSize.y);
      glfwMakeContextCurrent(m_threadWin);

      m_fullscreen = monitor != nullptr;
   }

   ~Impl()
   {
      glfwDestroyWindow(m_threadWin);
      glfwDestroyWindow(m_window);
      glfwTerminate();
   }

   void makeContextCurrent()
   {
      glfwMakeContextCurrent(m_window);

      auto err = glewInit();
      if(err != GLEW_OK)
      {
         const char *e = (const char*)glewGetErrorString(err);
         throw ArqException(e);
      }
   }

   void checkJoystickPresence()
   {
      for(int i = 0; i < GLFW_JOYSTICK_COUNT; ++i)
      {
         auto &c = m_controllers[i];

         if(glfwJoystickPresent(i))
         {
            if(!c.attached)
            {
               //register new controller and send callbacks
               c.attached = true;
               c.name = (char*)glfwGetJoystickName(i);

               int buttonCount;
               auto buttons = glfwGetJoystickButtons(i, &buttonCount);
               c.buttons.clear();
               c.buttons.resize(buttonCount, false);           

               int axisCount;
               auto axes = glfwGetJoystickAxes(i, &axisCount);
               c.axes.clear();
               c.axes.resize(axisCount * 2, false);  

               Logs::i(InputTag) << "Controller " << i << " connected";
               if(m_cbConPres)
                  (*m_cbConPres)(i, true);
            }
         }
         else
         {
            if(c.attached)
            {
               //controller has disconnected, send callback
               c.attached = false;
               Logs::i(InputTag) << "Controller " << i << " disconnected";
               if(m_cbConPres)
                  (*m_cbConPres)(i, false);
            }
         }
      }
   }

   int getKeyState(int key)
   {
      return KeyStates[key];
   }
   int getJoystickButtonState(int id, int button)
   {
      auto &c = m_controllers[id];
      return c.attached ? c.buttons[button] : 0;

   }
   int getJoystickAxisState(int id, int axis)
   {
      auto &c = m_controllers[id];
      return c.attached ? c.axes[axis] : 0;
   }

   void pollJoystickEvents()
   {
      if(m_controllerPresenceCheck == 0)
         checkJoystickPresence();

      if(m_controllerPresenceCheck++ > 300)
         m_controllerPresenceCheck = 0;

      for(int i = 0; i < GLFW_JOYSTICK_COUNT; ++i)
      {
         auto &c = m_controllers[i];

         //Logs::d(InputTag) << "Controller " << i << " Controller present: " << (int)&c;
         if(c.attached)
         {
            //controller is already registered, do buttons/axes
            int buttonCount;
            auto buttons = glfwGetJoystickButtons(i, &buttonCount);
               
            for(int j = 0; j < buttonCount; ++j)
            {
               bool pressed = buttons[j] != 0;

               if(pressed != c.buttons[j])
               {
                  Logs::i(InputTag) << "Controller " << i << " button " << j << (c.buttons[j] ? " released" : " pressed");
                  if(m_cbConButton)
                     (*m_cbConButton)(i, j, c.buttons[j] ? GLFW_RELEASE : GLFW_PRESS);

                  c.buttons[j] = pressed;
               }
            }

            int axisCount;
            //Logs::d(InputTag) << "Controller " << i << " Retreiving axes";
            auto axes = glfwGetJoystickAxes(i, &axisCount);
            for(int j = 0; j < axisCount; ++j)
            {
               bool pressedPos = axes[j] > JoystickThreshold;
               bool pressedNeg = axes[j] < -JoystickThreshold;
                  
               auto &negAxis = c.axes[j * 2];
               auto &posAxis = c.axes[j * 2 + 1];

               if(pressedNeg != negAxis)
               {
                  Logs::i(InputTag) << "Controller " << i << " negaxis " << j*2 << (negAxis ? " released" : " pressed");
                  if(m_cbConAxis)
                     (*m_cbConAxis)(i, j*2, negAxis ? GLFW_RELEASE : GLFW_PRESS);

                  negAxis = pressedNeg;
               }

               if(pressedPos != posAxis)
               {
                  Logs::i(InputTag) << "Controller " << i << " posaxis " << j*2+1 << (posAxis ? " released" : " pressed");
                  if(m_cbConAxis)
                     (*m_cbConAxis)(i, j*2+1, posAxis ? GLFW_RELEASE : GLFW_PRESS);

                  posAxis = pressedPos;
               }
            }
         }
      }

   }

   void pollEvents()
   {
      glfwPollEvents();
      pollJoystickEvents();
   }

   void swapBuffers()
   {
      glfwSwapBuffers(m_window);
   }

   int shouldClose()
   {
      return glfwWindowShouldClose(m_window);
   }

   Int2 getSize()
   {
      return m_windowSize;
   }

   Float2 getMousePosition()
   {
      double x, y;
      glfwGetCursorPos(m_window, &x, &y);

      return Float2((float)x, (float)y);
   }
   void setMousePosition(const Float2 &pos)
   {
      glfwSetCursorPos(m_window, pos.x, pos.y);
   }

   int getKey(int key)
   {
      return glfwGetKey(m_window, key);
   }
   int getMouseButton(int button)
   {
      return glfwGetMouseButton(m_window, button);
   }

   void setMouseButtonCallback(MouseButtonCallback cb)
   {
      _CBbtn = cb;
      glfwSetMouseButtonCallback(m_window, _mButton);
   }
   void setMousePositionCallback(MousePositionCallback cb)
   {
      _CBpos = cb;
      glfwSetCursorPosCallback(m_window, _mPosition);
   }
   void setMouseEnterCallback(MouseEnterCallback cb)
   {
      _CBenter = cb;
      glfwSetCursorEnterCallback(m_window, _mEnter);
   }
   void setMouseScrollCallback(MouseScrollCallback cb)
   {
      _CBscrl = cb;
      glfwSetScrollCallback(m_window, _mScroll);
   }
   void setKeyCallback(KeyCallback cb)
   {
      _CBkey = cb;
      glfwSetKeyCallback(m_window, _key);
   }
   void setCharCallback(CharCallback cb)
   {
      _CBchar = cb;
      glfwSetCharCallback(m_window, _char);
   }

   const char *getControllerName(int id)
   {
      if(id < GLFW_JOYSTICK_COUNT && m_controllers[id].attached)
         return m_controllers[id].name;

      return nullptr;
   }
   void setControllerPresenceCallback(ControllerPresenceCallback cb) { m_cbConPres = cb;}
   void setControllerButtonCallback(ControllerButtonCallback cb) { m_cbConButton = cb;}
   void setControllerAxisCallback(ControllerAxisCallback cb) { m_cbConAxis = cb;}

};


GLWindow::GLWindow(Int2 winSize, std::string windowName, GLFWmonitor *monitor)
   :pImpl(new Impl(winSize, windowName, monitor))
{}
GLWindow::~GLWindow(){}

void GLWindow::makeContextCurrent(){pImpl->makeContextCurrent();}
void GLWindow::pollEvents(){pImpl->pollEvents();}
void GLWindow::swapBuffers(){pImpl->swapBuffers();}
int GLWindow::shouldClose(){return pImpl->shouldClose();}
Int2 GLWindow::getSize(){return pImpl->getSize();}
Float2 GLWindow::getMousePosition(){return pImpl->getMousePosition();}
void GLWindow::setMousePosition(const Float2 &pos){pImpl->setMousePosition(pos);}
int GLWindow::getKey(int key){return pImpl->getKey(key);}
int GLWindow::getMouseButton(int button){return pImpl->getMouseButton(button);}
void GLWindow::setMouseButtonCallback(MouseButtonCallback cb){pImpl->setMouseButtonCallback(cb);}
void GLWindow::setMousePositionCallback(MousePositionCallback cb){pImpl->setMousePositionCallback(cb);}
void GLWindow::setMouseEnterCallback(MouseEnterCallback cb){pImpl->setMouseEnterCallback(cb);}
void GLWindow::setMouseScrollCallback(MouseScrollCallback cb){pImpl->setMouseScrollCallback(cb);}
void GLWindow::setKeyCallback(KeyCallback cb){pImpl->setKeyCallback(cb);}
void GLWindow::setCharCallback(CharCallback cb){pImpl->setCharCallback(cb);}
const char *GLWindow::getControllerName(int id){return pImpl->getControllerName(id);}
void GLWindow::setControllerPresenceCallback(ControllerPresenceCallback cb){pImpl->setControllerPresenceCallback(cb);}
void GLWindow::setControllerButtonCallback(ControllerButtonCallback cb){pImpl->setControllerButtonCallback(cb);}
void GLWindow::setControllerAxisCallback(ControllerAxisCallback cb){pImpl->setControllerAxisCallback(cb);}
int GLWindow::getKeyState(int key){return pImpl->getKeyState(key);}
int GLWindow::getJoystickButtonState(int id, int button){return pImpl->getJoystickButtonState(id, button);}
int GLWindow::getJoystickAxisState(int id, int axis){return pImpl->getJoystickAxisState(id, axis);}


