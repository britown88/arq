#pragma once

#include <memory>
#include "StringTable.h"
#include "Texture.h"

class TextureManager
{
   class Impl;
   std::unique_ptr<Impl> pImpl;

   friend class Texture;
   void acquire(Texture *tex);

public:
   TextureManager();
   ~TextureManager();

   Texture *getTexture(const TextureRequest &tex);   
};
