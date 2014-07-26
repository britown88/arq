#pragma once

struct MouseEvent
{
   enum class Type : unsigned int
   {
      Move = 0,
      Enter,
      Button,
      Scroll
   };

   Type type;
   bool entered;
   int button, action, mods;
   double x, y, scrollX, scrollY;

   MouseEvent(Type type)
      :type(type), entered(false), button(0), action(0), mods(0),
      x(0), y(0), scrollX(0), scrollY(0)
   {
   }

   bool operator==(const MouseEvent &rhs)const
   {
      return type == rhs.type && entered == rhs.entered && button == rhs.button && 
         action == rhs.action && mods == rhs.mods;
   }

   size_t hash() const
   {
      size_t h = 5381;

      h = (h << 5) + (h << 1) + (unsigned int)type;
      if(type == Type::Enter)
         h = (h << 5) + (h << 1) + entered;
      else if(type == Type::Button)
      {
         h = (h << 5) + (h << 1) + button;
         h = (h << 5) + (h << 1) + action;
         h = (h << 5) + (h << 1) + mods;
      }

      return h;
   }
};