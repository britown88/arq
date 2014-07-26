#pragma once

#include <memory>
#include "StringTable.h"
#include "Vector.h"

enum class RepeatType : unsigned int
{
   Repeat = 0,
   Clamp
};

enum class FilterType : unsigned int
{
   Linear = 0,
   Nearest
};

struct TextureRequest
{
   RepeatType repeatType;
   FilterType filterType;
   InternString path;

   TextureRequest(InternString path, RepeatType repeat = RepeatType::Clamp, FilterType filter = FilterType::Nearest)
      :path(path), repeatType(repeat), filterType(filter) {}

   bool operator==(const TextureRequest &rhs)const
   {
      return repeatType == rhs.repeatType && filterType == rhs.filterType && path == rhs.path;
   }

   size_t hash() const
   {
      size_t h = 5381;

      h = (h << 5) + (h << 1) + (unsigned int)repeatType;
      h = (h << 5) + (h << 1) + (unsigned int)filterType;
      h = (h << 5) + (h << 1) + (unsigned int)path;

      return h;
   }
   
};

class Texture
{
   class Impl;
   std::unique_ptr<Impl> pImpl;
   
   friend class TextureManager;

   void acquire();
   void release();

   bool isLoaded();

public:
   Texture(const TextureRequest &request);
   ~Texture();

   Int2 getSize();
   unsigned int getGLHandle();   
};