#include "GameMath.h"

namespace GameMath
{
   const float PI = 3.14159265359f;
   const float ToRadian = PI / 180.0f;
   const float Epsilon = 0.001f;

   Float2 perp2D(const Float2 &vec)
   {
      return Float2(-vec.y, vec.x);
   }

   float dot(const Float2 &vec1, const Float2 &vec2)
   {
      return vec1.x * vec2.x + vec1.y * vec2.y;
   }


   Float2 vectorBetweenBoxes(Rectf const& b1, Rectf const& b2)
   {
      Float2 out;
      //X dist
      if (b1.left > b2.right) 
         out.x = b2.right - b1.left;
      else if (b2.left > b1.right) 
         out.x = b2.left - b1.right;

      //Y dist
      if (b1.top > b2.bottom) 
         out.y = b2.bottom - b1.top;
      else if (b2.top > b1.bottom) 
         out.y = b2.top - b1.bottom;

      return out;
   }

   //self isn't offset.  This is assuming we know it'll have a collision...
   float offsetCollisionTime(Rectf& self, Float2 const& velocity, Rectf const& collide, Float2 &normal)
   {
      //distance until collision, find distance between boxes on X and Y
      auto vec = vectorBetweenBoxes(self, collide);
            
      float collisionTime = 0.0f;

      float xCollisionTime = 0.0f;
      if (velocity.x != 0.0f) 
         xCollisionTime = vec.x / velocity.x;
      else if (vec.x != 0.0f)
         xCollisionTime = 10000000.0f;

      float yCollisionTime = 0.0f;
      if (velocity.y != 0.0f) 
         yCollisionTime = vec.y / velocity.y;
      else if (vec.y != 0.0f)
         yCollisionTime = 10000000.0f;

      if (xCollisionTime < 0.0f) xCollisionTime = 1000000.0f;
      if (yCollisionTime < 0.0f) yCollisionTime = 1000000.0f;

      if(xCollisionTime > yCollisionTime)
      {
         collisionTime = xCollisionTime;
         normal = Float2(velocity.x < 0 ? 1.0f : -1.0f, 0.0f);
      }
      else
      {
         collisionTime = yCollisionTime;
         normal = Float2(0.0f, velocity.y < 0 ? 1.0f : -1.0f);
      }

      return collisionTime;
   }
  
};