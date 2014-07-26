#pragma once

#include <cstdio>

template<typename T>
class Color
{
public:
   T r, g, b, a;
   Color(T r, T g, T b, T a):a(a), r(r), g(g), b(b){}
   Color(T r, T g, T b):a((T)1), r(r), g(g), b(b){}
   Color():a((T)1), r((T)0), g((T)0), b((T)0){}
};

typedef Color<float> Colorf;
typedef unsigned char byte;


class Color255
{
public:
   byte r, g, b, a;
   Color255(byte r, byte g, byte b, byte a):r(r), g(g), b(b), a(a){}
   Color255(byte r, byte g, byte b):r(r), g(g), b(b), a(255){}
   Color255():r(0), g(0), b(0), a(0){}

};

struct Color255Hash
{
   std::size_t operator()(const Color255 c) const
   {
      return *(int*)&c;
   }
};

struct Color255Comp
{
   bool operator()(const Color255 lhs, const Color255 rhs) const
   {
      return *(int*)&lhs == *(int*)&rhs;
   }
};

