#pragma once

#include "Rect.h"
#include "Vector.h"

class Matrix
{
   float m_elements[16];
public:
   Matrix();

   const float *data() const {return m_elements;}

   float &operator[](size_t i){return m_elements[i];}

   Matrix operator*(const Matrix &rhs);
   Float3 operator*(const Float3 &rhs);
};



namespace MatrixTransforms
{
   void identity(Matrix &m);
   void ortho(Matrix &m, float left, float right, float bottom, float top, float near, float far);
   void scale(Matrix &m, float x, float y, float z = 1.0f);
   void translate(Matrix &m, float x, float y, float z = 0.0f);
   void rotate(Matrix &m, float angle);
   void rotateAround(Matrix &m, float angle, const Float3 &vector);

   void toRect(Matrix &m, const Rectf &r);
   void toRotatedRect(Matrix &m, const Rectf &r, float angle, const Float2 &rotPoint);

   void make2D(Matrix &m);
};

Matrix IdentityMatrix();