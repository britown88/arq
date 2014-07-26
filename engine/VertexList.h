#pragma once

#include "Vector.h"
#include "Color.h"

#include <bitset>
#include <vector>
#include <memory>

#include "boost\optional.hpp"
#include "ShaderManager.h"


template<ShaderAttribute vc>
struct VertexComponentType;

template<>
struct VertexComponentType<ShaderAttribute::Position>
{
   typedef Float2 type;
};

template<>
struct VertexComponentType<ShaderAttribute::Color>
{
   typedef Colorf type;
};

template<>
struct VertexComponentType<ShaderAttribute::TextureCoordinate>
{
   typedef Float2 type;
};

inline size_t sizeOfComponent(ShaderAttribute vc)
{
   switch(vc)
   {
   case ShaderAttribute::Position:
      return sizeof(VertexComponentType<ShaderAttribute::Position>::type);
   case ShaderAttribute::Color:
      return sizeof(VertexComponentType<ShaderAttribute::Color>::type);
   case ShaderAttribute::TextureCoordinate:
      return sizeof(VertexComponentType<ShaderAttribute::TextureCoordinate>::type);
   }

   return 0;
}

typedef std::bitset<(size_t)ShaderAttribute::COUNT> VertexFlags;

class VertexBuilder
{
   char *m_bytes;
   size_t *m_componentOffsets;
public:
   VertexBuilder(char *bytes, size_t *componentOffsets):m_bytes(bytes), m_componentOffsets(componentOffsets){}

   template<ShaderAttribute vc>
   VertexBuilder &with(typename VertexComponentType<vc>::type const &comp)
   {
      memcpy(m_bytes + m_componentOffsets[(int)vc], &comp, sizeof(comp));
      return *this;
   }
};

class VertexIterator
{
   VertexFlags m_flags;
   size_t m_vertexSize;
   char *m_first, *m_last;
   size_t *m_offsets;

public:
   VertexIterator(VertexFlags flags, size_t vertexSize, char *first, char *last, size_t *offsets):
      m_flags(flags), m_vertexSize(vertexSize), m_first(first), m_last(last), m_offsets(offsets)
   {}

   bool hasMore()
   {
      return m_first != m_last;
   }

   void moveNext()
   {
      m_first += m_vertexSize;
   }

   template<ShaderAttribute vc>
   typename VertexComponentType<vc>::type *get()
   {
      return m_flags[static_cast<vint>(vc)] ? (typename VertexComponentType<vc>::type*)(m_first + m_offsets[static_cast<vint>(vc)]) : nullptr;
   }
};

class VertexList
{
   VertexFlags m_flags;
   size_t m_vertexSize;
   size_t m_componentOffset[(unsigned int)ShaderAttribute::COUNT];

   std::vector<char> m_bytes;

public:
   VertexList(VertexFlags flags):m_flags(flags){build();}

   VertexList(const VertexList &rhs):
      m_flags(VertexFlags(rhs.m_flags)), m_vertexSize(rhs.m_vertexSize),
      m_bytes(std::vector<char>(rhs.m_bytes))
   {
      for(unsigned int i = 0; i < (unsigned int)ShaderAttribute::COUNT; ++i)
         m_componentOffset[i] = rhs.m_componentOffset[i];
   }

   VertexList(VertexList && ref):
      m_flags(std::move(ref.m_flags)), m_vertexSize(ref.m_vertexSize),
      m_bytes(std::move(ref.m_bytes))
   {
      for(unsigned int i = 0; i < (unsigned int)ShaderAttribute::COUNT; ++i)
         m_componentOffset[i] = ref.m_componentOffset[i];
   }

   bool has(ShaderAttribute attr)
   {
      return m_flags[(unsigned int)attr];
   }

   size_t getOffset(ShaderAttribute attr)
   {
      return m_componentOffset[(unsigned int)attr];
   }

   char *getData()
   {
      return m_bytes.data();
   }

   size_t size()
   {
      return m_bytes.size();
   }

   int getStride()
   {
      return m_vertexSize;
   }

   void build()
   {
      m_vertexSize = 0;

      for(unsigned int i = 0; i < (unsigned int)ShaderAttribute::COUNT; ++i)
      {
         m_componentOffset[i] = m_vertexSize;

         if(m_flags[i])
            m_vertexSize += sizeOfComponent((ShaderAttribute)i);
      }
   }

   VertexIterator iterate()
   {
      return VertexIterator(m_flags, m_vertexSize, m_bytes.data(), m_bytes.data() + m_bytes.size(), m_componentOffset);
   }

   VertexBuilder addVertex()
   {
      m_bytes.resize(m_bytes.size() + m_vertexSize);

      VertexBuilder vb(m_bytes.data() + m_bytes.size() - m_vertexSize, m_componentOffset);
      return vb;
   }

   VertexIterator operator[](unsigned int i)
   {
      return VertexIterator(m_flags, m_vertexSize, m_bytes.data() + (i* m_vertexSize), m_bytes.data() + m_bytes.size(), m_componentOffset);
   }
};

struct VBOAttribute
{
   int offset;
   int size;
};

class VBO
{
public:
   virtual const int getHandle() const=0;
   virtual const int getStride() const=0;
   virtual boost::optional<VBOAttribute> getAttribute(ShaderAttribute attr) const=0;
};

typedef std::shared_ptr<VBO> VBOPtr;

std::shared_ptr<VBO> createVBO(VertexList &vList);

class VertexListFactory
{
   VertexListFactory(){}

   VertexFlags m_flags;

   friend VertexListFactory createVertexList();
public:

   VertexListFactory &with(ShaderAttribute attr)
   {
      m_flags[(unsigned int)attr] = true;
      return *this;
   }

   VertexList build()
   {
      return VertexList(m_flags);
   }

};

inline VertexListFactory createVertexList()
{
   return VertexListFactory();
}

//createVertexList().with(VertexComponent::Position).build();

