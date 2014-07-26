#pragma once

#include <memory>

#include "StringTable.h"
#include "Matrix.h"

enum class ShaderUniform : unsigned int
{
   View = 0,
   Model,
   Color,
   TexCoord,
   Texture,
   COUNT
};

enum class ShaderAttribute : unsigned int
{
   Position=0,
   Color,
   TextureCoordinate,
   COUNT
};

class ShaderManager
{
   class Impl;
   std::unique_ptr<Impl> pImpl;
public:
   ShaderManager();
   ~ShaderManager();

   void buildShaders();
   bool built();

   void setAttribute(ShaderAttribute attr, int dataSize, int stride, int offset);
   void setUniform(ShaderUniform u, const float *data);

   void useShader(InternString name);
   void bindTextureUniform();

};