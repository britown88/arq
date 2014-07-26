#include "GL\glew.h"
#include "GLFW\glfw3.h"
#include "RenderThread.h"

#include "IOCContainer.h"
#include "GLWindow.h"
#include "DrawBuffer.h"
#include "ShaderManager.h"
#include "GnUtilities.h"
#include "Renderer.h"

#include <vector>
#include <mutex>
#include <thread>
#include <atomic>

class RenderThread::Impl
{
   std::atomic_bool m_isRunning;
   std::mutex m_mutex;
   std::thread m_thread;

   ShaderManager *sm;
   DrawBuffer *db;
   Renderer *r;
   GLWindow *w;

public:
   Impl(Renderer *r, ShaderManager *sm, DrawBuffer *db, GLWindow *w):r(r), sm(sm), db(db), w(w)
   {
      m_isRunning = false;
   }
   ~Impl(){}

   void run()
   {
      w->makeContextCurrent();
      sm->buildShaders();

      while(true)
      {
         auto &renderer = *r;
         glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
         glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

         auto queuePtr = db->get();
         auto &queue = *queuePtr;

         for(auto &obj : queue)
            obj->draw(renderer);

         postOGLError("RenderThread");

         //glFinish();
         w->swapBuffers();

         if(!m_isRunning)
            break;
      }
   }

   

   void start()
   {
      m_isRunning = true;
      m_thread = std::thread(&RenderThread::Impl::run, this);
   }

   void stop()
   {
      m_isRunning = false;
      m_thread.join();
   }
};


RenderThread::RenderThread(Renderer *r, ShaderManager *sm, DrawBuffer *db, GLWindow *w):pImpl(new Impl(r, sm, db, w)){}
RenderThread::~RenderThread(){}

void RenderThread::start(){pImpl->start();}
void RenderThread::stop(){pImpl->stop();}