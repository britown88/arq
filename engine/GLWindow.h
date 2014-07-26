#pragma once

struct GLFWwindow;
struct GLFWmonitor;

#include <string>
#include "Vector.h"
#include <vector>
#include <memory>

#include <functional>

class GLWindow
{
   class Impl;
   std::unique_ptr<Impl> pImpl;

public:
   typedef std::function<void(int button, int action, int mods, double x, double y)> MouseButtonCallback;
   typedef std::function<void(double x, double y)> MousePositionCallback;
   typedef std::function<void(bool entered, double x, double y)> MouseEnterCallback;
   typedef std::function<void(double offX, double offY, int mods, double posX, double posY)> MouseScrollCallback;
   typedef std::function<void(int key, int action, int mods)> KeyCallback;
   typedef std::function<void(unsigned int character)> CharCallback;

   GLWindow(Int2 winSize, std::string windowName, GLFWmonitor *monitor = nullptr);
   ~GLWindow();

   void makeContextCurrent();

   void pollEvents();
   void swapBuffers();
   int shouldClose();

   Int2 getSize();
   Float2 getMousePosition();
   void setMousePosition(const Float2 &pos);

   int getKey(int key);
   int getMouseButton(int button);

   void setMouseButtonCallback(MouseButtonCallback cb);
   void setMousePositionCallback(MousePositionCallback cb);
   void setMouseEnterCallback(MouseEnterCallback cb);
   void setMouseScrollCallback(MouseScrollCallback cb);
   void setKeyCallback(KeyCallback cb);
   void setCharCallback(CharCallback cb);

};
