#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <memory>
#include <cstdlib>
#include <assert.h>
#include <iostream>
#include "PropertyMap.h"

inline std::string readFile(std::string const& fileName)
{

   std::ifstream f(fileName);
   if (!f.is_open()) return "";

   f.seekg(0, f.end);
   size_t fSize = (size_t)f.tellg();
   f.seekg(0, f.beg);

   std::string out;
   out.resize(fSize);
   f.read(&out[0], fSize);

   return out;
}

inline void writeFile(std::string const& fileName, std::string const& data)
{
   std::ofstream f(fileName,std::ios_base::out | std::ios_base::trunc);
   if (!f.is_open())
   {
      return; //throw?
   }
   f.write(data.c_str(), data.size());
}
struct ObjType
{
   enum e_ObjType
   {
      Array,
      Object,
      Int,
      Double,
      Boolean,
      String
   };
};

typedef ObjType::e_ObjType e_ObjType;


class JObj
{
public:
   virtual ~JObj(){}  //so that vectors and stuff get their destructors called.  Kinda cheesy, fix it later perhaps.
   e_ObjType objType;
};


class JArrayObj : public JObj
{
public:
   JArrayObj() {objType = ObjType::Array;}
   std::vector<std::unique_ptr<JObj> > children;
};


class JObjectObj : public JObj
{
public:
   JObjectObj() {objType = ObjType::Object;}
   std::vector<std::pair<std::unique_ptr<JObj>, std::unique_ptr<JObj> > > children;
};

class JIntObj : public JObj
{
public:
   JIntObj() {objType = ObjType::Int;}
   int value;
};

class JDoubleObj : public JObj
{
public:
   JDoubleObj() {objType = ObjType::Double;}
   double value;
};

class JBooleanObj : public JObj
{
public:
   JBooleanObj() {objType = ObjType::Boolean;}
   bool value;
};


class JStringObj : public JObj
{
public:
   JStringObj() {objType = ObjType::String;}
   std::string value;
};
inline std::string getString(std::unique_ptr<JObj>& obj)
{
	return static_cast<JStringObj*>(obj.get())->value;
}
inline int getInt(std::unique_ptr<JObj>& obj)
{
	return static_cast<JIntObj*>(obj.get())->value;
}

inline float getFloat(std::unique_ptr<JObj>& obj)
{
	return (float)static_cast<JDoubleObj*>(obj.get())->value;
}

class JSONReader
{
   std::string file;
   size_t readPos;
   std::vector<std::unique_ptr<JObj> > children;

   void skipWhiteSpace()
   {
      for (;;)
      {
         if (endOfFile()) return;
         auto ch = file[readPos];
         if (ch == ' ' || ch == '\n' || ch == '\t' || ch == 0) 
         {
            ++readPos;
            continue;
         }
         break;
      }
   }
   void rewind(int count)
   {
      readPos -= count;
   }
   std::unique_ptr<JObj> readNumeric()
   {
      std::string numericStr;
      char ch = readCh();
      bool decimal = false;
      while (ch >= '0' && ch <= '9' || ch == '-' || ch == '.')
      {
         if (ch == '.') decimal = true;
         numericStr += ch;
         ch = readCh();
      }
      rewind(1);

      if (decimal)
      {
         //double
         JDoubleObj* d = new JDoubleObj;
         char* endPtr = (char*)(numericStr.c_str() + numericStr.size());
         d->value = strtod(numericStr.c_str(), &endPtr);
         return std::unique_ptr<JObj>(d);
      }
      JIntObj* i = new JIntObj;
      i->value = atoi(numericStr.c_str());
      return std::unique_ptr<JObj>(i);
   }
   bool endOfFile()
   {
      return readPos == file.size();
   }
   char readCh()
   {
      return file[readPos++];
   }
   std::unique_ptr<JObj> readObj()
   {
      skipWhiteSpace();
      if (endOfFile()) return nullptr;

      auto ch = readCh();

      //skip unread terminators
      if (ch == ':' || ch == ',')
      {
         skipWhiteSpace();
         if (endOfFile()) return nullptr;
         ch = readCh();
      }
      if (ch == '}' || ch == ']') return nullptr; //end obj/array.

      //aggregates
      if (ch == '[') //array
      {
         JArrayObj* arrayObj = new JArrayObj();
         while (auto obj = readObj())
         {
            arrayObj->children.push_back(std::move(obj));
         }
         return std::unique_ptr<JObj>(arrayObj);
      }
      if (ch == '{') //dict
      {
         JObjectObj* object = new JObjectObj();
         for (;;)
         {
            auto obj1 = readObj();
            if (!obj1) return std::unique_ptr<JObj>(object);
            auto obj2 = readObj();
            if (!obj2) return std::unique_ptr<JObj>(object);

            object->children.push_back(std::make_pair(std::move(obj1), std::move(obj2)));
         }
      }

      //ok, it's an actual value type.
      if (ch >= '0' && ch <= '9' || ch == '-' || ch == '.') //numeric
      {
         rewind(1);
         return readNumeric();
      }
      else if (ch == '\"') //string
      {
         JStringObj* strObj = new JStringObj;
         for (;;)
         {
            ch = readCh();
            if (ch == '\"')
            {
               break;
            }
            else if (ch == '\\')
            {
               ch = readCh();
            }
            strObj->value.push_back(ch);
         }
         return std::unique_ptr<JObj>(strObj);
      }
      if (ch == 't') //true and false
      {
         auto out = new JBooleanObj();
         out->value = true;
         return std::unique_ptr<JObj>(out);
      } 
      else if (ch == 'f')
      {
         auto out = new JBooleanObj();
         out->value = false;
         return std::unique_ptr<JObj>(out);
      }

      assert(NULL && "Unknown type.");
      return nullptr; //wat
   }
public:
   JSONReader (std::string fileName) : readPos(0)
   {
      file = readFile(fileName);
      while (auto obj = readObj())
      {
         children.push_back(std::move(obj));
      }
   }
   JSONReader (std::string file_in, int) : readPos(0)
   {
      file = std::move(file_in);
      while (auto obj = readObj())
      {
         children.push_back(std::move(obj));
      }
   }
   std::vector<std::unique_ptr<JObj> > get()
   {
      return std::move(children);
   }
};

inline std::vector<std::unique_ptr<JObj> > getJSONObjects(std::string const& fileName)
{
   return JSONReader(fileName).get();
}

inline Property JSONObjectToProperty(JObj& obj)
{
   //recursive as shit!
   switch(obj.objType)
   {
   case ObjType::Array:
   {
      auto& arr = static_cast<JArrayObj&>(obj);
      PropertyArray pArray;
      for (auto& child : arr.children)
      {
         pArray.push_back(JSONObjectToProperty(*child));
      }
      return pArray;
   }
   case ObjType::Object:
   {
      PropertyMap pMap;
      auto& jobj = static_cast<JObjectObj&>(obj);
      for (auto& child : jobj.children)
      {
         auto key = *JSONObjectToProperty(*child.first).get<std::string>();
         auto value = JSONObjectToProperty(*child.second);
         pMap.insert(std::make_pair(std::move(key), std::move(value)));
      }
      return pMap;
   }
   case ObjType::Int:
      return static_cast<JIntObj&>(obj).value;
   case ObjType::Double:
      return (float)static_cast<JDoubleObj&>(obj).value;
   case ObjType::Boolean:
      return static_cast<JBooleanObj&>(obj).value;
   case ObjType::String:
      return static_cast<JStringObj&>(obj).value;
   }
   return Property();
}


class JSONWriter
{
   struct State
   {
      State() : inObject(false), keyWritten(false), firstWritten(false)
      {

      }
      bool inObject;
      bool keyWritten;
      bool firstWritten;
   };
   std::vector<State> stateStack;
   std::string output;


   void writeIndent()
   {

      //need to add indent.
      size_t indentSize = 3 * (stateStack.size()-1);
      for (size_t i = 0; i < indentSize; ++i)
      {
         output += " ";
      }
   }
   void writePrefix()
   {
      auto& state = stateStack.back();
      if (state.firstWritten)
      {
         output += ",\n";
      }
      else
      {
         state.firstWritten = true;
      }

      if (state.inObject && state.keyWritten) return;
      writeIndent();
   }
   void writeSuffix()
   {
      auto& state = stateStack.back();
      if (state.inObject)
      {
         if (!state.keyWritten)
         {
            output += " : ";
            state.firstWritten = false;
         }
         state.keyWritten = !state.keyWritten;
      }
   }
public:
   JSONWriter()
   {
      stateStack.push_back(State());
   }

   void write(int val)
   {
      writePrefix();

      char buf[256];
      sprintf(buf, "%d", val);
      output += buf;

      writeSuffix();
   }
   void write(bool val)
   {
      writePrefix();

      output += val ? 't' : 'f';

      writeSuffix();
   }
   void write(double val)
   {
      writePrefix();

      char buf[256];
      sprintf(buf, "%f", val);
      output += buf;

      writeSuffix();
   }
   void write (std::string const& val)
   {
      writePrefix();
      output += "\"";
      output += val;
      output += "\"";
      writeSuffix();
   }

   void startArray()
   {
      writePrefix();

      stateStack.push_back(State());
      output += "[\n";
   }
   void endArray()
   {
      stateStack.pop_back();
      output += "\n";
      writeIndent();
      output += "]";



      writeSuffix();
   }
   void startObject()
   {
      writePrefix();

      stateStack.push_back(State());
      stateStack.back().inObject = true;
      output += "{\n";
   }
   void endObject()
   {
      stateStack.pop_back();
      output += "\n";
      writeIndent();
      output += "}";
      writeSuffix();
   }
   std::string getOutput()
   {
      return std::move(output);
   }
};

inline void writeProperty(JSONWriter& writer, Property& p)
{
   switch(p.type())
   {
   case PropertyMapType::Integer:
      writer.write(*p.get<int>());
      break;
   case PropertyMapType::Float: 
      writer.write((double)*p.get<float>());
      break;
   case PropertyMapType::String:
      writer.write(*p.get<std::string>());
      break;
   case PropertyMapType::Boolean:
      writer.write(*p.get<bool>());
      break;
   case PropertyMapType::PropertyMap:
      {
         writer.startObject();
         std::vector<PropertyMap::value_type*> values;
         for (auto& val : *p.get<PropertyMap>())
         {
            values.push_back(&val);
         }
         std::sort(begin(values), end(values), [](PropertyMap::value_type* lhs, PropertyMap::value_type* rhs){
            return lhs->first < rhs->first;
         });

         for (auto obj : values)
         {
            writer.write(obj->first);
            writeProperty(writer, obj->second);
         }
         writer.endObject();
      }
      break;
   case PropertyMapType::Array:
      writer.startArray();
      for (auto& obj : *p.get<PropertyArray>())
      {
         writeProperty(writer, obj);
      }
      writer.endArray();
      break;
   }
}

inline Property loadJSONToProperty(std::string const& fileName)
{
   auto objs = getJSONObjects(fileName);

   if (objs.size() == 1)
   {
      return JSONObjectToProperty(*objs.back());
   }
   PropertyArray out;
   for (auto& obj : objs)
   {
      out.push_back(JSONObjectToProperty(*obj));
   }
   return out;
}
inline PropertyArray asArray(Property& p)
{
   if (p.type() != PropertyMapType::Array)
   {
      PropertyArray out;
      out.push_back(p);
      return out;
   }
   return *p.get<PropertyArray>();
}


inline Property JSONStringToProperty(std::string str)
{
   //garbage constructor I suck cocks - SP
   auto objs = JSONReader(str, 0).get();

   if (objs.size() == 1)
   {
      return JSONObjectToProperty(*objs.back());
   }
   PropertyArray out;
   for (auto& obj : objs)
   {
      out.push_back(JSONObjectToProperty(*obj));
   }
   return out;
}

inline std::string propertyToJSONString(Property& p)
{
   JSONWriter out;
   writeProperty(out, p);
   return out.getOutput();
}