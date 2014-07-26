#include "GL\glew.h"
#include "GLFW\glfw3.h"

#include "DrawObjects.h"

#include "IOCContainer.h"
#include "Renderer.h"
#include "StringTable.h"
#include "PagedAllocator.h"
#include "Texture.h"
#include "GameHelpers.h"

class DrawTexture : public IRenderable
{
   VBOPtr vbo;
   IBOPtr ibo;
   Matrix transform, texTransform;
   Colorf colorTransform;
   Texture *tex;
public:
   DrawTexture(VBOPtr vbo, IBOPtr ibo, Texture *tex, 
      const Matrix &transform, const Matrix &texTransform, const Colorf &colorTransform)
      :vbo(vbo), ibo(ibo), tex(tex), transform(transform), texTransform(texTransform), colorTransform(colorTransform)
   {
   }

   void *operator new(size_t count)
   {
      return PagedAllocator<DrawTexture>::instance().alloc();
   }

   void operator delete(void *ptr)
   {
      PagedAllocator<DrawTexture>::instance().free((DrawTexture*)ptr);
   }

   void draw(Renderer &r)
   {
      static InternString shader = IOC.resolve<StringTable>()->get("texture");

      r.useShader(shader);
      r.setUniformMatrix(ShaderUniform::Model, transform);
      r.setUniformMatrix(ShaderUniform::TexCoord, texTransform);
      r.setUniformMatrix(ShaderUniform::Color, colorTransform);

      r.setAttributes(*vbo);

      r.bindTexture(tex->getGLHandle());

      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

      r.drawElements(GL_TRIANGLES, *ibo);

      glDisable(GL_BLEND);

   }
};

std::unique_ptr<IRenderable> DrawObjects::texture(VBOPtr vbo, IBOPtr ibo, Texture *tex, 
   const Matrix &transform, const Matrix &texTransform, const Colorf &colorTransform)
{
   return std::unique_ptr<IRenderable>(new DrawTexture(vbo, ibo, tex, transform, texTransform, colorTransform));
}

class DrawText : public IRenderable
{
   static VBOPtr vbo;
   static IBOPtr ibo;
   Matrix transform;
   Colorf colorTransform, shadowColor;
   TextString text;
public:
   DrawText(const TextString &text, const Matrix &transform, 
      const Colorf &colorTransform)
      :text(text), transform(transform), colorTransform(colorTransform), shadowColor()
   {
   }

   void *operator new(size_t count)
   {
      return PagedAllocator<DrawTexture>::instance().alloc();
   }

   void operator delete(void *ptr)
   {
      PagedAllocator<DrawTexture>::instance().free((DrawTexture*)ptr);
   }

   void draw(Renderer &r)
   {
      static InternString shader = IOC.resolve<StringTable>()->get("texture");
      auto fe = IOC.resolve<FontEngine>();
      auto &font = fe->getFont(*text.font);
      auto &&chars = font.getCharacters();

      r.useShader(shader);
      //only set vbo once
      r.setUniformMatrix(ShaderUniform::Color, shadowColor);
      r.setAttributes(*vbo);

      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

      Matrix currentChar = transform;
      MatrixTransforms::translate(currentChar, 2.0f, 2.0f);
      for(char c : text.text)
      {
         r.setUniformMatrix(ShaderUniform::Model, currentChar * chars[c].model);
         r.setUniformMatrix(ShaderUniform::TexCoord, chars[c].texCoord);
         r.bindTexture(chars[c].texture);

         r.drawElements(GL_TRIANGLES, *ibo);
         MatrixTransforms::translate(currentChar, chars[c].width, 0.0f);         
      }  

      r.setUniformMatrix(ShaderUniform::Color, colorTransform);

      currentChar = transform;
      for(char c : text.text)
      {
         r.setUniformMatrix(ShaderUniform::Model, currentChar * chars[c].model);
         r.setUniformMatrix(ShaderUniform::TexCoord, chars[c].texCoord);
         r.bindTexture(chars[c].texture);

         r.drawElements(GL_TRIANGLES, *ibo);
         MatrixTransforms::translate(currentChar, chars[c].width, 0.0f);         
      } 

      glDisable(GL_BLEND);

   }
};

VBOPtr DrawText::vbo = GameHelpers::standardRectangleVBO();
IBOPtr DrawText::ibo = GameHelpers::standardRectangleIBO();

std::unique_ptr<IRenderable> DrawObjects::text(const TextString &text, const Matrix &transform, 
      const Colorf &colorTransform)
{
   return std::unique_ptr<IRenderable>(new DrawText(text, transform, colorTransform));
}


class DrawTriangles : public IRenderable
{
   VBOPtr vbo;
   IBOPtr ibo;
   Matrix transform;
   Colorf colorTransform;
public:
   DrawTriangles(VBOPtr vbo, IBOPtr ibo, const Matrix &transform, const Colorf &colorTransform)
      :vbo(vbo), ibo(ibo), transform(transform), colorTransform(colorTransform)
   {
   }

   void *operator new(size_t count)
   {
      return PagedAllocator<DrawTriangles>::instance().alloc();
   }

   void operator delete(void *ptr)
   {
      PagedAllocator<DrawTriangles>::instance().free((DrawTriangles*)ptr);
   }

   void draw(Renderer &r)
   {
      static InternString shader = IOC.resolve<StringTable>()->get("color");

      r.useShader(shader);
      r.setUniformMatrix(ShaderUniform::Model, transform);
      r.setUniformMatrix(ShaderUniform::Color, colorTransform);

      r.setAttributes(*vbo);

      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

      r.drawElements(GL_TRIANGLES, *ibo);

      glDisable(GL_BLEND);

   }
};

std::unique_ptr<IRenderable> DrawObjects::triangles(VBOPtr vbo, IBOPtr ibo, 
   const Matrix &transform, const Colorf &colorTransform)
{
   return std::unique_ptr<IRenderable>(new DrawTriangles(vbo, ibo, transform, colorTransform));
}

class DrawViewport : public IRenderable
{
   Rectf bounds;
public:
   DrawViewport(const Rectf &bounds):bounds(bounds){}

   void *operator new(size_t count)
   {
      return PagedAllocator<DrawViewport>::instance().alloc();
   }

   void operator delete(void *ptr)
   {
      PagedAllocator<DrawViewport>::instance().free((DrawViewport*)ptr);
   }

   void draw(Renderer &r)
   {
      glViewport((int)bounds.left, (int)bounds.top, (int)bounds.right, (int)bounds.bottom);
   }
};

std::unique_ptr<IRenderable> DrawObjects::viewport(const Rectf &bounds)
{
   return std::unique_ptr<IRenderable>(new DrawViewport(bounds));
}


class DrawCamera : public IRenderable
{
   Matrix m;
public:
   DrawCamera(const Rectf &bounds)
   {
      MatrixTransforms::ortho(m, 
         bounds.left, bounds.right, 
         bounds.bottom, bounds.top, 
         1.0f, -1.0f);
   }

   void *operator new(size_t count)
   {
      return PagedAllocator<DrawCamera>::instance().alloc();
   }

   void operator delete(void *ptr)
   {
      PagedAllocator<DrawCamera>::instance().free((DrawCamera*)ptr);
   }

   void draw(Renderer &r)
   {
      r.setUniformMatrix(ShaderUniform::View, m);
   }
};

std::unique_ptr<IRenderable> DrawObjects::camera(const Rectf bounds)
{
   return std::unique_ptr<IRenderable>(new DrawCamera(bounds));
}


class DrawScissorOn : public IRenderable
{
   Rectf bounds;
public:
   DrawScissorOn(const Rectf &bounds):bounds(bounds){}

   void *operator new(size_t count)
   {
      return PagedAllocator<DrawScissorOn>::instance().alloc();
   }

   void operator delete(void *ptr)
   {
      PagedAllocator<DrawScissorOn>::instance().free((DrawScissorOn*)ptr);
   }

   void draw(Renderer &r)
   {
      glScissor((int)bounds.left, (int)bounds.top, (int)bounds.right, (int)bounds.bottom);
      glEnable(GL_SCISSOR_TEST);
   }
};

std::unique_ptr<IRenderable> DrawObjects::scissor(const Rectf bounds)
{
   return std::unique_ptr<IRenderable>(new DrawScissorOn(bounds));
}


class DrawScissorOff : public IRenderable
{
public:
   void *operator new(size_t count)
   {
      return PagedAllocator<DrawScissorOff>::instance().alloc();
   }

   void operator delete(void *ptr)
   {
      PagedAllocator<DrawScissorOff>::instance().free((DrawScissorOff*)ptr);
   }

   void draw(Renderer &r)
   {
      glDisable(GL_SCISSOR_TEST);
   }
};

std::unique_ptr<IRenderable> DrawObjects::scissorOff()
{
   return std::unique_ptr<IRenderable>(new DrawScissorOff());
}