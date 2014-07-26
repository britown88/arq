#pragma once

#include <memory>

#include "VertexList.h"
#include "IndexList.h"
#include "Matrix.h"
#include "Rect.h"
#include "Texture.h"
#include "Text.h"

class IRenderable;

namespace DrawObjects
{
   std::unique_ptr<IRenderable> triangles(VBOPtr vbo, IBOPtr ibo, 
      const Matrix &transform, const Colorf &colorTransform);
   std::unique_ptr<IRenderable> texture(VBOPtr vbo, IBOPtr ibo, Texture *tex, 
      const Matrix &transform, const Matrix &texTransform, const Colorf &colorTransform);
   std::unique_ptr<IRenderable> text(const TextString &text, const Matrix &transform, 
      const Colorf &colorTransform);

   std::unique_ptr<IRenderable> viewport(const Rectf &bounds);
   std::unique_ptr<IRenderable> camera(const Rectf bounds);
   std::unique_ptr<IRenderable> scissor(const Rectf bounds);
   std::unique_ptr<IRenderable> scissorOff();

};



