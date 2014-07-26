#include <GL/glew.h>

#include "VertexList.h"
#include "boost\optional.hpp"

std::shared_ptr<VBO> createVBO(VertexList &vList)
{
   class _VBO : public VBO
   {
      int m_bufferHandle, m_stride;
      boost::optional<VBOAttribute> m_attributes[(unsigned int)ShaderAttribute::COUNT];

      mutable boost::optional<VertexList> m_vList;
   public:
      _VBO(VertexList &vList):m_vList(vList)
      {
         m_stride = vList.getStride();

         for(unsigned int i = 0; i < (unsigned int)ShaderAttribute::COUNT; ++i)
         {
            auto vc = (ShaderAttribute)i;
            if(vList.has(vc))
            {
               VBOAttribute vboa;
               vboa.offset = vList.getOffset(vc);
               vboa.size = sizeOfComponent(vc);

               m_attributes[i] = vboa;
            }
         }

      }

      ~_VBO()
      {
         //glDeleteBuffers(1, (const GLuint*)&m_bufferHandle);
      }

      const int getHandle() const
      {
         if(m_vList)
         {
            glGenBuffers(1, (GLuint*)&m_bufferHandle);

            glBindBuffer(GL_ARRAY_BUFFER, m_bufferHandle);
            glBufferData(GL_ARRAY_BUFFER, m_vList->size(), m_vList->getData(), GL_STATIC_DRAW);
			   glBindBuffer(GL_ARRAY_BUFFER, 0);

            m_vList = boost::none;
         }

         return m_bufferHandle;
      }

      const int getStride() const
      {
         return m_stride;
      }

      boost::optional<VBOAttribute> getAttribute(ShaderAttribute vc) const
      {
         return m_attributes[(unsigned int)vc];
      }
   };

   return std::make_shared<_VBO>(vList);

}