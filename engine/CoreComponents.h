#pragma once

#include "Component.h"
#include "Entity.h"

#include "Vector.h"
#include "IndexList.h"
#include "VertexList.h"
#include "Color.h"
#include "boost\optional.hpp"
#include "StringTable.h"
#include "Matrix.h"
#include "Texture.h"

void registerCoreComponents();

struct PositionComponent : public Component
{
   Float2 pos;
   PositionComponent(){}
   PositionComponent(Float2 pos):pos(pos){}
};

struct MeshComponent : public Component
{
   IBOPtr ibo;
   VBOPtr vbo;
   Colorf c;

   MeshComponent():c(1, 1, 1){}
   MeshComponent(IBOPtr ibo, VBOPtr vbo, Colorf c):ibo(ibo), vbo(vbo), c(c){}
};

struct GraphicalBoundsComponent : public Component
{
   Float2 size, center;
   GraphicalBoundsComponent(){}
   GraphicalBoundsComponent(Float2 size):size(size){}
   GraphicalBoundsComponent(Float2 size, Float2 center):size(size), center(center){}
};

struct RotationComponent : public Component
{
   float angle;
   Float2 point;
   RotationComponent():angle(0.0f){}
   RotationComponent(float angle):angle(angle){}
   RotationComponent(float angle, Float2 point)
      :angle(angle), point(point){}
};

struct LayerComponent : public Component
{
   int layer;
   LayerComponent(){}
   LayerComponent(int layer):layer(layer){}
};

struct TextureComponent : public Component
{
   InternString filePath;
   int blendS, blendD;
   bool flipX, flipY;
   boost::optional<Float2> size;
   FilterType filterType;
   RepeatType repeatType;

   //sets blend values to default,
   //used for CPP implemenetation
   void defaultBlend();
   TextureComponent(){}
   TextureComponent(InternString filePath):
      flipX(false), flipY(false), filePath(filePath),
      filterType(FilterType::Nearest), repeatType(RepeatType::Clamp)
   {
      defaultBlend();
   }
};