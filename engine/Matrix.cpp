#include "Matrix.h"
#include "GameMath.h"

#include <string.h>
#include <math.h> 

Matrix::Matrix()
{
}

Matrix Matrix::operator*(const Matrix &rhs)
{
   Matrix m;
   for(int y = 0; y < 4; ++y)
   {
      float v1[4] = {m_elements[y], m_elements[y + 4], m_elements[y + 8], m_elements[y + 12]};

      for(int x = 0; x < 4; ++x)
      {
         const float *v2 = &(rhs.m_elements[x * 4]);

         float v = 0.0f;
         for(int i = 0; i < 4; ++i)
            v += v1[i] * v2[i];

         m[x*4 + y] = v;
      }
   }

   return m;
   
}

Float3 Matrix::operator*(const Float3 &rhs)
{
   Float3 out;

   out.x = rhs.x * m_elements[0] + rhs.y * m_elements[4] + rhs.z * m_elements[8 ] + m_elements[12];
   out.y = rhs.x * m_elements[1] + rhs.y * m_elements[5] + rhs.z * m_elements[9 ] + m_elements[13];
   out.z = rhs.x * m_elements[2] + rhs.y * m_elements[6] + rhs.z * m_elements[10] + m_elements[14];

   return out;
}

void MatrixTransforms::ortho(
   Matrix &m, float left, float right, float bottom, 
   float top, float near, float far)
{
   m[1] = m[2] = m[3] = m[4] = m[6] = m[7] = m[8] = m[9] = m[11] = 0;
   
   m[0 ] = 2.0f / (right - left);
   m[5 ] = 2.0f / (top - bottom);
   m[10] = -2.0f / (far - near);
   m[15] = 1.0f;

   m[12] = -((right + left) / (right - left));
   m[13] = -((top + bottom) / (top - bottom));
   m[14] = -((far + near) / (far - near));
}

void MatrixTransforms::identity(Matrix &m)
{
   memset((void*)m.data(), 0, 16 * sizeof(float));
   m[0] = 1.0f;
   m[5] = 1.0f;
   m[10] = 1.0f;
   m[15] = 1.0f;
}

void MatrixTransforms::scale(Matrix &m, float x, float y, float z)
{
    Matrix scale;
    identity(scale);
    scale[0] = x;
    scale[5] = y;
    scale[10] = z;
    m = m * scale;
}

void MatrixTransforms::translate(Matrix &m, float x, float y, float z)
{
   Matrix trans;
   identity(trans);
   trans[12] = x;
   trans[13] = y;
   trans[14] = z;
   m = m * trans;
}

void MatrixTransforms::rotate(Matrix &m, float angle)
{
   float a = angle * GameMath::ToRadian;
   float c = cosf(a);
   float s = sinf(a);

   Matrix rot;
   identity(rot);

   rot[0] = c;
   rot[1] = s;
   rot[4] = -s;
   rot[5] = c;

   m = m * rot;
}

void MatrixTransforms::rotateAround(Matrix &m, float angle, const Float3 &vector)
{
   float a = angle * GameMath::ToRadian;
   float c = cosf(a);
   float s = sinf(a);

   float x = vector.x;
   float y = vector.y;
   float z = vector.z;

   Matrix rot;
   identity(rot);

   rot[0] = (x*x)*(1-c)+(c  ); rot[4] = (x*y)*(1-c)-(z*s); rot[8 ] = (x*z)*(1-c)+(y*s);
   rot[1] = (y*x)*(1-c)+(z*s); rot[5] = (y*y)*(1-c)+(c  ); rot[9 ] = (y*z)*(1-c)-(x*s);
   rot[2] = (x*z)*(1-c)-(y*s); rot[6] = (y*z)*(1-c)+(x*s); rot[10] = (z*z)*(1-c)+(c  );

   m = m * rot;
}

void MatrixTransforms::toRect(Matrix &m, const Rectf &r)
{
   identity(m);
   translate(m, r.left, r.top);   
   scale(m, r.width(), r.height());
}
void MatrixTransforms::toRotatedRect(Matrix &m, const Rectf &r, float angle, const Float2 &rotPoint)
{
   auto rot = rotPoint;
   rot.x *= r.width();
   rot.y *= r.height();

   identity(m);
   translate(m, r.left, r.top);  
   translate(m, rot.x, rot.y);  
   rotate(m, angle);
   translate(m, -rot.x, -rot.y); 
   scale(m, r.width(), r.height());
}

void MatrixTransforms::make2D(Matrix &m)
{
   m[2] = m[6] = m[8] = m[9] = m[11] = m[14] = 0.0f;
   m[10] = 1.0f;

   float x = m[0];
   float y = m[1];

   float len = x * x + y * y;
   if(len == 0.0f)
   {
      m[0] = 1.0f;
      m[1] = 0.0f;
   }
   else
   {
      len = 1.0f / sqrt(len);
      m[0] *= len;
      m[1] *= len;
   }

   x = m[4];
   y = m[5];

   len = x * x + y * y;
   if(len == 0.0f)
   {
      m[5] = 1.0f;
      m[4] = 0.0f;
   }
   else
   {
      len = 1.0f / sqrt(len);
      m[5] *= len;
      m[4] *= len;
   }
}

Matrix IdentityMatrix()
{
   Matrix m;
   MatrixTransforms::identity(m);
   return m;
}


