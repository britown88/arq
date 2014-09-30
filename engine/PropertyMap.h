#pragma once

#include "Vector.h"
#include "boost/optional.hpp"
#include <vector>
#include <unordered_map>

static const size_t PropertySize=32;

struct PropertyMapType {
   enum e_PropertyMapType {
   Empty, Integer, Float, String, Boolean, PropertyMap, Array,
   COUNT
   };
};

typedef PropertyMapType::e_PropertyMapType e_PropertyMapType;

class IPropertyType
{
public:
   ~IPropertyType() {}
   virtual void copy(void* ptr, void const* data)=0;
   virtual void destruct(void* ptr)=0;
   virtual void* get(void const* data)=0;
   virtual e_PropertyMapType getType()=0;
};

template <typename T>
struct PropMapTypeLookup{};

template <typename T>
static e_PropertyMapType getPropertyMapType(PropMapTypeLookup<T>)
{
   static_assert(false, "unsupported property map type");
}


template <typename T>
class PropertyMapTypeImpl : public IPropertyType
{
public:
   static PropertyMapTypeImpl& instance()
   {
      static PropertyMapTypeImpl inst;
      return inst;
   }
   virtual void copy(void* ptr, void const* data)
   {
      if (sizeof(T)>PropertySize)
      {
         new (ptr) std::shared_ptr<T>(std::make_shared<T>(*(T*)data));
      }
      else
      {
         new (ptr) T(*(T*)data);
      }
   }
   virtual void destruct(void* ptr)
   {
      if (sizeof(T)>PropertySize)
      {
        ((std::shared_ptr<T>*)ptr)->~shared_ptr();
      }
      else
      {
         ((T*)ptr)->~T();
      }
   }
   virtual void* get(void const* data)
   {
      if (sizeof(T)>PropertySize)
      {
         return ((std::shared_ptr<T>*)data)->get();
      }
      return (void*)data;
   }
   virtual e_PropertyMapType getType()
   {
      return getPropertyMapType(PropMapTypeLookup<T>());
   }
};

class Property
{
public:
   Property() : propType(nullptr)
   {
      
   }
   ~Property()
   {
      clear();
   }
   Property(Property const& rhs) : propType(rhs.propType)
   {
      if (!empty()) propType->copy(data.buffer, propType->get(rhs.data.buffer));
   }
   template <typename T>
   Property(T const& data) : propType(&PropertyMapTypeImpl<T>::instance())
   {
      propType->copy(this->data.buffer, &data);
   }
   template <typename T>
   Property& operator=(T const& data)
   {
      clear();
      propType = &PropertyMapTypeImpl<T>::instance();
      propType->copy(this->data.buffer, &data);
      return *this;
   }
   Property& operator=(Property const& rhs)
   {
      if (this == &rhs) return *this;
      clear();
      propType = rhs.propType;
      if (!empty()) propType->copy(data.buffer, propType->get(rhs.data.buffer));
      return *this;
   }
   template <typename T>
   T* get()
   {
      
      if (type() != PropertyMapTypeImpl<T>::instance().getType()) return nullptr;
      return (T*)propType->get(data.buffer);
   }
   e_PropertyMapType type()
   {
      if (empty()) return PropertyMapType::Empty;
      return propType->getType();
   }
   bool empty() const
   {
      return propType == 0;
   }
   void clear()
   {
      if (!empty())
      {
         propType->destruct(data.buffer);
      }
   }
   inline Property& operator[](std::string const&);
private:
   union
   {
      char buffer[PropertySize];
      double alignmentHackMemberCPPElevenFullSupportWhenVolvoPlzFix;
   } data;
   IPropertyType* propType;
};


typedef std::unordered_map<std::string, Property> PropertyMap;
typedef std::vector<Property> PropertyArray;

static e_PropertyMapType getPropertyMapType(PropMapTypeLookup<int>){return PropertyMapType::Integer;}
static e_PropertyMapType getPropertyMapType(PropMapTypeLookup<float>){return PropertyMapType::Float;}
static e_PropertyMapType getPropertyMapType(PropMapTypeLookup<std::string>){return PropertyMapType::String;}
static e_PropertyMapType getPropertyMapType(PropMapTypeLookup<bool>){return PropertyMapType::Boolean;}
static e_PropertyMapType getPropertyMapType(PropMapTypeLookup<PropertyMap>){return PropertyMapType::PropertyMap;}
static e_PropertyMapType getPropertyMapType(PropMapTypeLookup<PropertyArray>){return PropertyMapType::Array;}


Property& Property::operator[](std::string const& str)
{
   return (*get<PropertyMap>())[str];
}

template <typename T>
T getPropertyOr(PropertyMap& pMap, std::string const& key, T defaultValue)
{
   auto iter = pMap.find(key);
   if (iter == pMap.end()) return std::move(defaultValue);
   if (auto val = iter->second.get<T>())
   {
      return *val;
   }
   return std::move(defaultValue);
}

template <typename T>
boost::optional<T&> getSubproperty(PropertyMap& pMap, std::string key)
{
   auto iter = pMap.find(key);
   if (iter != pMap.end()) 
   {
      if (auto val = iter->second.get<T>())
      {
         return *val;
      }
   }
   return boost::none;
}