#pragma once

#include <mutex>
#include <vector>
#include <memory>

#include "GnUtilities.h"
#include "Renderer.h"
#include "DrawObjects.h"

class DrawBuffer
{
   typedef std::unique_ptr<IRenderable> ObjPtr;
   typedef std::vector<ObjPtr> ObjList;
   
   std::shared_ptr<ObjList> m_workingQueue, m_drawQueue;

   std::mutex m_mutex;

public:
   DrawBuffer()
      :m_workingQueue(new ObjList()), 
      m_drawQueue(new ObjList())
   {
   }

   //adds a draw object to the current working queue
   void add(ObjPtr obj)
	{
		m_workingQueue->push_back(std::move(obj));
	}
		
   //swaps the working queue with the drawing queue
	void swap()
	{	
      m_mutex.lock();
      m_drawQueue = std::move(m_workingQueue);
      m_workingQueue.reset(new ObjList());
      m_mutex.unlock();
	}

   //gets the drawing queue
	std::shared_ptr<ObjList> get() 
	{
      m_mutex.lock();
      auto out = m_drawQueue;
      m_mutex.unlock();
		return out;
	}
};