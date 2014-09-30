#pragma once

#include "Vector.h"
#include "Rect.h"

namespace GameMath
{
   extern const float ToRadian;
   extern const float PI;
   extern const float Epsilon;

   Float2 vectorBetweenBoxes(Rectf const& b1, Rectf const& b2);
   float offsetCollisionTime(Rectf& self, Float2 const& velocity, Rectf const& collide, Float2 &normal);

   Float2 perp2D(const Float2 &vec);

   float dot(const Float2 &vec1, const Float2 &vec2);
};