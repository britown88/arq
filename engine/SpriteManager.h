#pragma once

#include <unordered_map>
#include "StringTable.h"
#include "Animation.h"
#include "TextureManager.h"
#include "PropertyMap.h"

typedef IAnimation<Texture*> ISpriteAnimation;

std::unique_ptr<ISpriteAnimation> buildSpriteAnimation(Property &p);

class SpriteManager
{
   std::unordered_map<InternString, std::unique_ptr<ISpriteAnimation>> m_sprites;
public:

   void addSprite(std::string name, std::unique_ptr<ISpriteAnimation> anim);
   ISpriteAnimation *getSprite(InternString name);
};