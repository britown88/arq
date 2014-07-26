#pragma once


template <typename T>
class Vector2
{
public:
   Vector2() : x((T)0), y((T)0) {}
   Vector2(T x_in, T y_in) : x(x_in), y(y_in) {}
   T x,y;

   Vector2<T> operator+(const Vector2<T> &rhs){ return Vector2<T>(x+rhs.x, y+rhs.y);}
   Vector2<T> operator-(const Vector2<T> &rhs){ return Vector2<T>(x-rhs.x, y-rhs.y);}
   Vector2<T> &operator+=(const Vector2<T> &rhs){ x+=rhs.x; y+=rhs.y; return *this;}
   Vector2<T> operator*(const T scalar){ return Vector2<T>(x*scalar, y*scalar);}
   Vector2<T> operator*=(const Vector2<T> &rhs){ x*=rhs.x; y*=rhs.y; return *this;}
   Vector2<T> operator*(const Vector2<T> &rhs){ return Vector2<T>(x*rhs.x, y*rhs.y);}
   Vector2<T> operator/(const Vector2<T> &rhs){ return Vector2<T>(x/rhs.x, y/rhs.y);}

   bool operator==(const Vector2<T> &rhs){ return x==rhs.x && y==rhs.y;}
   bool operator!=(const Vector2<T> &rhs){ return x!=rhs.x || y!=rhs.y;}

};

typedef Vector2<float> Float2;
typedef Vector2<int> Int2;


template <typename T>
class Vector3
{
public:
   Vector3() : x((T)0), y((T)0), z((T)0) {}
   Vector3(T x_in, T y_in, T z_in) : x(x_in), y(y_in), z(z_in) {}
   T x,y,z;

   Vector3<T> operator+(const Vector3<T> &rhs){ return Vector3<T>(x+rhs.x, y+rhs.y, z+rhs.z);}
   Vector3<T> operator-(const Vector3<T> &rhs){ return Vector3<T>(x-rhs.x, y-rhs.y, z-rhs.z);}
   Vector3<T> &operator+=(const Vector3<T> &rhs){ x+=rhs.x; y+=rhs.y; z+=rhs.z; return *this;}
   Vector3<T> operator*(const T scalar){ return Vector3<T>(x*scalar, y*scalar, z*scalar);}
   Vector3<T> operator*=(const Vector3<T> &rhs){ x*=rhs.x; y*=rhs.y; z*=rhs.z; return *this;}
   Vector3<T> operator*(const Vector3<T> &rhs){ return Vector3<T>(x*rhs.x, y*rhs.y, z*rhs.z);}
   Vector3<T> operator/(const Vector3<T> &rhs){ return Vector3<T>(x/rhs.x, y/rhs.y, z/rhs.z);}

   bool operator==(const Vector3<T> &rhs){ return x==rhs.x && y==rhs.y && z==rhs.z;}
   bool operator!=(const Vector3<T> &rhs){ return x!=rhs.x || y!=rhs.y || z!=rhs.z;}

};

typedef Vector3<float> Float3;
typedef Vector3<int> Int3;

