#include "GL/glew.h"
#include <GLFW/glfw3.h>
#include "GLWindow.h"

#include "GnUtilities.h"

GLWindow::MouseButtonCallback _CBbtn;
GLWindow::MousePositionCallback _CBpos;
GLWindow::MouseEnterCallback _CBenter;
GLWindow::MouseScrollCallback _CBscrl;
GLWindow::KeyCallback _CBkey;
GLWindow::CharCallback _CBchar;

void _mButton(GLFWwindow *win, int btn, int act, int mod)
{
   double x, y;
   glfwGetCursorPos(win, &x, &y);
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
void _key(GLFWwindow *win, int key, int scan, int act, int mod){_CBkey(key, act, mod);}
void _char(GLFWwindow *win, unsigned int c){_CBchar(c);}

class GLWindow::Impl
{
   GLFWwindow* m_window, *m_threadWin;
   Int2 m_windowSize;
   bool m_fullscreen;

public:
   Impl(Int2 winSize, std::string windowName, GLFWmonitor *monitor = nullptr)
   {
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

      m_fullscreen = monitor;
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

   void pollEvents()
   {
      glfwPollEvents();
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

      return Float2(x, y);
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


