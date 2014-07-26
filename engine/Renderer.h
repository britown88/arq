#pragma once

#include <memory>

#include "Rect.h"
#include "Matrix.h"
#include "boost\optional.hpp"
#include "VertexList.h"
#include "IndexList.h"
#include "StringTable.h"
#include "ShaderManager.h"
#include "DrawObjects.h"
#include "Color.h"
#include "GLWindow.h"
#include "Texture.h"

void postOGLError(std::string tag);

class Renderer;

class IRenderable
{
public:
   virtual ~IRenderable(){}
   virtual void draw(Renderer &r)=0;
};

class Renderer
{
   class Impl;
   std::unique_ptr<Impl> pImpl;
public:
   Renderer(GLWindow *w);
   ~Renderer();

   void terminate();

   //CPU Draw Calls
   void pushViewport(const Rectf &vpBounds);
   void popViewport();

   void pushCamera(const Rectf &camBounds);
   void popCamera();

   void pushScissor(const Rectf &scissorBounds);
   void popScissor();

   //swap the working drawqueue with the drawing one
   void swapBuffers();

   void drawTriangles(VBOPtr vbo, IBOPtr ibo, 
      const Matrix &transform, const Colorf &colorTransform);

   void drawTexture(VBOPtr vbo, IBOPtr ibo, Texture *tex,
      const Matrix &transform, const Matrix &texTransform, const Colorf &colorTransform);

   void drawText(const TextString &text, const Matrix &transform, const Colorf &colorTransform);
   
   //OpenGL Calls
   void setUniformMatrix(ShaderUniform u, const Matrix &m);
   void setUniformMatrix(ShaderUniform u, const Colorf &m);
   void setAttributes(const VBO &vbo);

   void drawElements(int mode, const IBO &ibo);
   void bindTexture(int handle);

   void useShader(InternString shaderName);

};