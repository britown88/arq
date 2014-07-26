#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Renderer.h"
#include "IOCContainer.h"
#include "Application.h"
#include "DrawBuffer.h"
#include "RenderThread.h"

#include <Windows.h>

#include <memory>

#include <sstream>
#include <stack>

class Renderer::Impl
{
   std::unique_ptr<RenderThread> m_renderThread;
   ShaderManager m_shaderManager;
   int m_lastTexture, m_lastVBO, m_lastIBO;
   DrawBuffer m_buffer;
   std::stack<Rectf> m_vpStack, m_camStack, m_scissorStack;
public:
   Impl(Renderer *r, GLWindow *w):m_lastTexture(-1), m_lastVBO(-1), m_lastIBO(-1)
   {
      m_renderThread.reset(new RenderThread(r, &m_shaderManager, &m_buffer, w));
      m_renderThread->start();
      while(!m_shaderManager.built()){}//wait for thread to build everything
   }

   void terminate()
   {
      m_renderThread->stop();
   }

   void pushViewport(const Rectf &vpBounds)
   {
      auto winSize = IOC.resolve<Application>()->windowSize();
      Rectf bounds(vpBounds.left, winSize.y - vpBounds.top - vpBounds.height(), vpBounds.width(), vpBounds.height());

      m_vpStack.push(bounds);
      m_buffer.add(DrawObjects::viewport(bounds));
   }
   void popViewport()
   {
      m_vpStack.pop();
      if(!m_vpStack.empty())
         m_buffer.add(DrawObjects::viewport(m_vpStack.top()));
   }

   void pushCamera(const Rectf &camBounds)
   {
      m_camStack.push(camBounds);
      m_buffer.add(DrawObjects::camera(camBounds));
   }
   void popCamera()
   {
      m_camStack.pop();
      if(!m_camStack.empty())
         m_buffer.add(DrawObjects::camera(m_camStack.top()));
   }

   void pushScissor(const Rectf &scissorBounds)
   {
      auto winSize = IOC.resolve<Application>()->windowSize();
      Rectf bounds(scissorBounds.left, winSize.y - scissorBounds.top - scissorBounds.height(), scissorBounds.width(), scissorBounds.height());
      
      m_scissorStack.push(bounds);
      m_buffer.add(DrawObjects::scissor(bounds));

   }
   void popScissor()
   {      
      m_scissorStack.pop();
      if(!m_scissorStack.empty())
         m_buffer.add(DrawObjects::scissor(m_scissorStack.top()));
      else
         m_buffer.add(DrawObjects::scissorOff());
   }

   void swapBuffers()
   {
      m_buffer.swap();
   }

   void drawTriangles(VBOPtr vbo, IBOPtr ibo, const Matrix &transform, const Colorf &colorTransform)
   {
      m_buffer.add(DrawObjects::triangles(vbo, ibo, transform, colorTransform));
   }

   void drawTexture(VBOPtr vbo, IBOPtr ibo, Texture *tex,
      const Matrix &transform, const Matrix &texTransform, const Colorf &colorTransform)
   {
      m_buffer.add(DrawObjects::texture(vbo, ibo, tex, transform, texTransform, colorTransform));
   }

   void drawText(const TextString &text, const Matrix &transform, const Colorf &colorTransform)
   {
      m_buffer.add(DrawObjects::text(text, transform, colorTransform));
   }
   
   //OpenGL Calls
   void setUniformMatrix(ShaderUniform u, const Matrix &m)
   {
      m_shaderManager.setUniform(u, m.data());
   }

   void setUniformMatrix(ShaderUniform u, const Colorf &c)
   {
      m_shaderManager.setUniform(u, (float *)&c);
   }

   void setAttributes(const VBO &vbo)
   {
      int handle = vbo.getHandle();
      if(handle != m_lastVBO)
      {
         m_lastVBO = handle;
         glBindBuffer(GL_ARRAY_BUFFER, handle);

         int stride = vbo.getStride();

         for(unsigned int i = 0; i < (unsigned int)ShaderAttribute::COUNT; ++i)
         {
            auto attr = (ShaderAttribute)i;
            if(auto attrData = vbo.getAttribute(attr))
               m_shaderManager.setAttribute(attr, attrData->size, stride, attrData->offset);
         }
      }
   }

   void drawElements(int mode, const IBO &ibo)
   {
      int handle = ibo.getHandle();
      if(handle != m_lastIBO)
      {
         m_lastIBO = handle;
         glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle);
      }

      glDrawElements(mode, ibo.getCount(), GL_UNSIGNED_BYTE, 0);
   }

   void bindTexture(int handle)
   {
      if(handle != m_lastTexture)
      {
         m_shaderManager.bindTextureUniform();
         glActiveTexture(GL_TEXTURE0);
         glBindTexture(GL_TEXTURE_2D, handle);
         
      }
   }

   void useShader(InternString shaderName)
   {
      m_shaderManager.useShader(shaderName);
   }
};

Renderer::Renderer(GLWindow *w)
   :pImpl(new Impl(this, w))
{
}

Renderer::~Renderer()
{
}

void Renderer::pushViewport(const Rectf &vpBounds){pImpl->pushViewport(vpBounds);}
void Renderer::popViewport(){pImpl->popViewport();}

void Renderer::pushCamera(const Rectf &camBounds){pImpl->pushCamera(camBounds);}
void Renderer::popCamera(){pImpl->popCamera();}

void Renderer::pushScissor(const Rectf &scissorBounds){pImpl->pushScissor(scissorBounds);}
void Renderer::popScissor(){pImpl->popScissor();}

void Renderer::swapBuffers(){pImpl->swapBuffers();}

void Renderer::drawTriangles(VBOPtr vbo, IBOPtr ibo, 
      const Matrix &transform, const Colorf &colorTransform)
{
   pImpl->drawTriangles(
      vbo, ibo, 
      transform, colorTransform);
}

void Renderer::drawTexture(VBOPtr vbo, IBOPtr ibo, Texture *tex,
      const Matrix &transform, const Matrix &texTransform, const Colorf &colorTransform)
{
   pImpl->drawTexture(
      vbo, ibo, tex,
      transform, texTransform, colorTransform);
}

void Renderer::drawText(const TextString &text, const Matrix &transform, const Colorf &colorTransform)
{
   pImpl->drawText(text, transform, colorTransform);
}
   
//OpenGL Calls
void Renderer::setUniformMatrix(ShaderUniform u, const Matrix &m){pImpl->setUniformMatrix(u, m);}
void Renderer::setUniformMatrix(ShaderUniform u, const Colorf &m){pImpl->setUniformMatrix(u, m);}
void Renderer::setAttributes(const VBO &vbo){pImpl->setAttributes(vbo);}

void Renderer::drawElements(int mode, const IBO &ibo){pImpl->drawElements(mode, ibo);}
void Renderer::bindTexture(int handle){pImpl->bindTexture(handle);}

void Renderer::useShader(InternString shaderName){pImpl->useShader(shaderName);}

void Renderer::terminate(){pImpl->terminate();}

void postOGLError(std::string tag)
{
   std::stringstream strim;

   strim << tag << " ";

	switch(glGetError())
	{		
	case GL_INVALID_ENUM:
		strim << "Invalid Enum";
		break;
	case GL_INVALID_VALUE:
		strim << "Invalid Value";
		break;
	case GL_INVALID_OPERATION:
		strim << "Invalid Operation";
		break;
	case GL_INVALID_FRAMEBUFFER_OPERATION:
		strim << "Invalid Framebuffer Operation";
		break;
	case GL_OUT_OF_MEMORY:
		strim << "Out of Memory";
		break;
	case GL_NO_ERROR:
	default:
		return;
	}

   MessageBox(0, strim.str().c_str(), "AROAR", MB_OK);

}