#pragma once

#include <memory>

class ShaderManager;
class DrawBuffer;
class Renderer;
class GLWindow;

class RenderThread
{
   class Impl;
   std::unique_ptr<Impl> pImpl;
public:
   RenderThread(Renderer *r, ShaderManager *sm, DrawBuffer *db, GLWindow *w);
   ~RenderThread();

   void start();
   void stop();


};