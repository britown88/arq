#pragma once

struct ControllerEvent
{
   enum class Type : unsigned int
   {
      Presence = 0,
      Button,
      Axis
   };

   Type type;

   int id;
   int buttonaxis, action;
   bool attached;

   ControllerEvent(Type type)
      :type(type), id(0), buttonaxis(0), action(0), attached(false)
   {
   }

   bool operator==(const ControllerEvent &rhs)const
   {
      return type == rhs.type && 
         (id == rhs.id || type == Type::Presence) && 
         buttonaxis == rhs.buttonaxis && 
         action == rhs.action && 
         attached ==  rhs.attached;
   }

   size_t hash() const
   {
      size_t h = 5381;

      h = (h << 5) + (h << 1) + (unsigned int)type;
      
      if(type == Type::Presence)
         h = (h << 5) + (h << 1) + (int)attached;
      else
      {
         h = (h << 5) + (h << 1) + (int)id;
         h = (h << 5) + (h << 1) + buttonaxis;
         h = (h << 5) + (h << 1) + action;
      }

      return h;
   }
};