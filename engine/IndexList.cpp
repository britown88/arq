#include <GL/glew.h>

#include "IndexList.h"
#include "boost\optional.hpp"

std::shared_ptr<IBO> createIBO(IndexList &iList)
{
   class _IBO : public IBO
   {
      int m_bufferHandle, m_size;

      mutable boost::optional<IndexList> m_iList;

   public:
      _IBO(IndexList &iList):m_iList(iList)
      {
         m_size = iList.indices.size();
      }

      ~_IBO()
      {
         //glDeleteBuffers(1, (const GLuint*)&m_bufferHandle);
      }

      int const getHandle() const
      {
         if(m_iList)
         {
            glGenBuffers(1, (GLuint*)&m_bufferHandle);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufferHandle);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_iList->indices.size(), m_iList->indices.data(), GL_STATIC_DRAW);
			   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

            m_iList = boost::none;
         }
         return m_bufferHandle;
      }

      int const getCount() const
      {
         return m_size;
      }
   };

   return std::make_shared<_IBO>(iList);

}