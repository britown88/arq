#include "SpriteManager.h"
#include "IOCContainer.h"
#include "AssetManager.h"
#include "SeanSort.h"
#include "Application.h"

std::unique_ptr<ISpriteAnimation> buildSpriteAnimation(Property &p)
{
   //get folder and fps
   if(auto folder = p["folder"].get<std::string>())
   if(auto fps = p["fps"].get<float>())
   {
      auto st = IOC.resolve<StringTable>();
      auto am = IOC.resolve<AssetManager>();

      std::vector<InternString> files;

      //loop over folder and create list of filenames
      for(auto file : am->getFiles(folder))
         files.push_back(st->get(file));

      if(!files.empty())
      {
         //sort list alphabetically
         files = seanSort(files, [&](const InternString &e1, const InternString &e2)->bool{return *e1 < *e2;});

         auto tm = IOC.resolve<TextureManager>();
         std::vector<Texture*> textures;
         //for each one, cvreate a texture request and push its texture* into a list
         for(auto file : files)
            textures.push_back(tm->getTexture(TextureRequest(file)));

         //pass list and fps into new ianim
         auto app = IOC.resolve<Application>();
         float msPerFrame = (app->frameTime()) / (*fps / app->frameRate());

         auto out = vectorToAnimation(std::move(textures), msPerFrame);

         //if looping make looping
         out = loopAnimation(std::move(out));

         return out;
      }
   }

   return nullptr;      
}

void SpriteManager::addSprite(std::string name, std::unique_ptr<ISpriteAnimation> anim)
{
   auto st = IOC.resolve<StringTable>();
   m_sprites.insert(std::make_pair(st->get(name), std::move(anim)));
}

ISpriteAnimation *SpriteManager::getSprite(InternString name)
{
   auto iter = m_sprites.find(name);
   if(iter != m_sprites.end())
      return iter->second.get();

   return nullptr;
}