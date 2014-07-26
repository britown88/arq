#include "TextureManager.h"
#include "Texture.h"
#include "GnUtilities.h"
#include  "ObjectHash.h"

#include <vector>
#include <unordered_map>

class TextureManager::Impl
{
   static const size_t TextureMemory = 104857600;//100mb
   static const size_t BitsPerPixel = 32;

   std::unordered_map<TextureRequest, 
      std::unique_ptr<Texture>, ObjectHash<TextureRequest>> m_textures;
   std::vector<Texture*> m_loadedTextures;
   size_t m_loadedBytes;
public:
   Impl():m_loadedBytes(0){}
   ~Impl(){}

   void acquire(Texture *tex)
   {
      if(tex->isLoaded())
      {
         //TODO used linked list for tex memory management

         //auto size = m_loadedTextures.size();
         //auto iter = std::find(m_loadedTextures.data(), m_loadedTextures.data() + size, tex);
         //size_t index = iter - m_loadedTextures.data();
         //
         //if(index == size)
         //   throw ArqException("Texture was loaded but not through the texture manager!");

      }
      else
      {
         tex->acquire();

         //TODO used linked list for tex memory management

         //auto dims = tex->getSize();
         //auto texSize = (BitsPerPixel * dims.x * dims.y) / 8.0f;
         //m_loadedTextures.push_back(tex);
         //m_loadedBytes += texSize;

         //while(m_loadedBytes > TextureMemory)
         //{
         //   auto unload = m_loadedTextures[0];
         //   auto udims = unload->getSize();
         //   auto utexSize = (BitsPerPixel * dims.x * dims.y) / 8.0f;
         //   unload->release();
         //   m_loadedTextures.erase(m_loadedTextures.begin());
         //   m_loadedBytes -= utexSize;
         //}
      }
   }

   Texture *getTexture(const TextureRequest &tex)
   {
      auto iter = m_textures.find(tex);
      if(iter == m_textures.end())
      {
         m_textures.insert(std::make_pair(tex, std::unique_ptr<Texture>(new Texture(tex))));
         iter = m_textures.find(tex);
      }

      return iter->second.get();
   }
};

void TextureManager::acquire(Texture *tex){pImpl->acquire(tex);}

TextureManager::TextureManager():pImpl(new Impl()){}
TextureManager::~TextureManager(){}

Texture *TextureManager::getTexture(const TextureRequest &tex){return pImpl->getTexture(tex);}
