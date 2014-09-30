#pragma once

struct KeyEvent
{
   enum class Type : unsigned int
   {
      Key = 0,
      Char
   };

   Type type;

   int key, action, mods;
   unsigned int character;

   KeyEvent(Type type)
      :type(type), key(0), action(0), mods(0), character(0)
   {
   }

   bool operator==(const KeyEvent &rhs)const
   {
      return type == rhs.type && key == rhs.key && 
         action == rhs.action && mods == rhs.mods && character == rhs.character;
   }

   size_t hash() const
   {
      size_t h = 5381;

      h = (h << 5) + (h << 1) + (unsigned int)type;
      if(type == Type::Char)
         h = (h << 5) + (h << 1) + character;
      else if(type == Type::Key)
      {
         h = (h << 5) + (h << 1) + key;
         h = (h << 5) + (h << 1) + action;
         h = (h << 5) + (h << 1) + mods;
      }

      return h;
   }
};