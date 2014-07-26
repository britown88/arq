#include "GameHelpers.h"

#include "Rect.h"
#include "Color.h"

VBOPtr _buildStandardRectVBO(const Colorf &c1, const Colorf &c2, const Colorf &c3, const Colorf &c4)
{
   Rectf rect(0, 0, 1, 1);

   auto vertices = createVertexList()
         .with(ShaderAttribute::Position)
         .with(ShaderAttribute::Color)
         .with(ShaderAttribute::TextureCoordinate)
         .build();

   vertices.addVertex()
      .with<ShaderAttribute::Position>(Float2(rect.left, rect.top))
      .with<ShaderAttribute::Color>(c1)
      .with<ShaderAttribute::TextureCoordinate>(Float2(0, 0));

   vertices.addVertex()
      .with<ShaderAttribute::Position>(Float2(rect.right, rect.top))
      .with<ShaderAttribute::Color>(c2)
      .with<ShaderAttribute::TextureCoordinate>(Float2(1, 0));

   vertices.addVertex()
      .with<ShaderAttribute::Position>(Float2(rect.right, rect.bottom))
      .with<ShaderAttribute::Color>(c3)
      .with<ShaderAttribute::TextureCoordinate>(Float2(1, 1));

   vertices.addVertex()
      .with<ShaderAttribute::Position>(Float2(rect.left, rect.bottom))
      .with<ShaderAttribute::Color>(c4)
      .with<ShaderAttribute::TextureCoordinate>(Float2(0, 1));

   return createVBO(vertices);
}

VBOPtr _buildStandardRectVBO(const Colorf &c)
{
   return _buildStandardRectVBO(c, c, c, c);
}

VBOPtr GameHelpers::standardRectangleVBO()
{
   static VBOPtr standardMesh = _buildStandardRectVBO(Colorf(1, 1, 1));
   return standardMesh;
}

VBOPtr GameHelpers::gradientRectangleVBO()
{
   static VBOPtr standardMesh = _buildStandardRectVBO(
      Colorf(1, 1, 1), 
      Colorf(1, 1, 1), 
      Colorf(0.15, 0.15, 0.15), 
      Colorf(0.15, 0.15, 0.15));
   return standardMesh;
}

IBOPtr _buildStandardRectIBO()
{
   IndexList il;
   il.indices.push_back(0);il.indices.push_back(1);il.indices.push_back(3);
   il.indices.push_back(1);il.indices.push_back(3);il.indices.push_back(2);

   return createIBO(il);
}

IBOPtr GameHelpers::standardRectangleIBO()
{
   static IBOPtr indices = _buildStandardRectIBO();
   return indices;
}