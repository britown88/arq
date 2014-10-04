#pragma once

#include "Vector.h"
#include <algorithm>

template<typename T>
class Rect2
{
public:
   Rect2() : top((T)0), bottom((T)0), left((T)0), right((T)0) {}
   Rect2(T left_in, T top_in, T right_in, T bot_in) : left(left_in), top(top_in), right(right_in), bottom(bot_in) {}
   T top, bottom, left, right;

   T width() const{return right - left;}
   T height() const{return bottom - top;}

   Rect2<T> operator*(const Rect2<T> &rhs)
   {
      return Rect2<T>(rhs.left*left, rhs.top*top, rhs.right*right, rhs.bottom*bottom);
   }

   bool contains(const Vector2<T> &pos) const
   {
      return pos.x >= left && pos.x < right && pos.y >= top && pos.y < bottom;
   }

   bool contains(const Rect2<T> &rect) const
   {
      if (left >= rect.right ||
         top >= rect.bottom ||
         rect.left >= right ||
         rect.top >= bottom) return false;
      return true;
   }

   Rect2<T> intersection(const Rect2<T> &rect) const
   {
      return Rect2<T>(
         std::max(left, rect.left),
         std::max(top, rect.top),
         std::min(right, rect.right),
         std::min(bottom, rect.bottom)
         );
   }

   void offset(const Vector2<T> &offset)
   {
      top += offset.y;
      bottom += offset.y;
      left += offset.x;
      right += offset.x;
   }  

   void offset(const T x, const T y)
   {
      top += y;
      bottom += y;
      left += x;
      right += x;
   } 

   void inset(const Vector2<T> &inset)
   {
      top += inset.y;
      bottom -= inset.y;

      left += inset.x;
      right -= inset.x;
   }
};

typedef Rect2<int> Recti;
typedef Rect2<float> Rectf;