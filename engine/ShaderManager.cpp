#include <GL\glew.h>

#include "ShaderManager.h"

#include <unordered_map>
#include <vector>
#include <fstream>
#include <sstream>
#include <atomic>

#include "IOCContainer.h"
#include "AssetManager.h"
#include "GnUtilities.h"

#include "boost\algorithm\string.hpp"

struct Shader
{
   std::string vertexPath, fragmentPath;
   int handle;
   std::vector<ShaderUniform> uniforms;
   std::vector<ShaderAttribute> attributes;
};

enum class UniformType : unsigned int
{
   mat4 = 0,
   vec4,
   sampler2D
};

struct UniformLookup
{
   UniformLookup(){}
   UniformLookup(ShaderUniform ID, std::string name, UniformType type, std::string typeName)
      :ID(ID), name(name), type(type), typeName(typeName)
   {}
   ShaderUniform ID;
   std::string name, typeName;
   UniformType type;
};

struct AttributeLookup
{
   AttributeLookup(){}
   AttributeLookup(ShaderAttribute ID, std::string name, std::string typeName)
      :ID(ID), name(name), typeName(typeName)
   {}
   ShaderAttribute ID;
   std::string name, typeName;
};

class ShaderManager::Impl
{
   int m_uniforms[(unsigned int)ShaderUniform::COUNT];
   UniformType m_uniformTypes[(unsigned int)ShaderUniform::COUNT];
   int m_attributes[(unsigned int)ShaderAttribute::COUNT];

   Matrix m_currentView;
   std::unordered_map<InternString, Shader> m_shaders;
   InternString m_currentShader;

   std::unordered_map<std::string, UniformLookup> m_uniformStringLookups;
   std::unordered_map<ShaderUniform, UniformLookup> m_uniformEnumLookups;
   std::unordered_map<std::string, AttributeLookup> m_attributeStringLookups;
   std::unordered_map<ShaderAttribute, AttributeLookup> m_attributeEnumLookups;

   void addUniformLookup(ShaderUniform ID, std::string name, UniformType type, std::string typeName)
   {
      UniformLookup l(ID, name, type, typeName);
      m_uniformStringLookups.insert(std::make_pair(name, l));
      m_uniformEnumLookups.insert(std::make_pair(ID, l));

      m_uniformTypes[(unsigned int)ID] = type;
   }

   void addAttributeLookup(ShaderAttribute ID, std::string name, std::string typeName)
   {
      AttributeLookup a(ID, name, typeName);
      m_attributeStringLookups.insert(std::make_pair(name, a));
      m_attributeEnumLookups.insert(std::make_pair(ID, a));
   }

   void buildLookups()
   {
      addUniformLookup(ShaderUniform::Model, "u_modelMatrix", UniformType::mat4, "mat4");
      addUniformLookup(ShaderUniform::View, "u_viewMatrix", UniformType::mat4, "mat4");
      addUniformLookup(ShaderUniform::TexCoord, "u_texMatrix", UniformType::mat4, "mat4");
      addUniformLookup(ShaderUniform::Color, "u_colorTransform", UniformType::vec4, "vec4");
      addUniformLookup(ShaderUniform::Texture, "u_texture", UniformType::sampler2D, "sampler2D");

      addAttributeLookup(ShaderAttribute::Position, "a_position", "vec2");
      addAttributeLookup(ShaderAttribute::Color, "a_color", "vec4");
      addAttributeLookup(ShaderAttribute::TextureCoordinate, "a_texCoords", "vec2");

   }

   void parseShaderFile(Shader &shader, std::string fileName)
   {
      std::ifstream file(fileName);

      if(!file.is_open())
         throw(ArqException("Unable to open shader file for parsing."));

      std::string line;
      std::vector<std::string> parts;
      while(std::getline(file, line))
      {
         //bail out once we hit main
         if(line.find("void main()") != std::string::npos)
         {
            file.close();
            return;
         }            

         auto semiColonIndex = line.find_last_of(';');
         if(semiColonIndex == std::string::npos)
            continue;

         line = line.substr(0, semiColonIndex);
         boost::split(parts, line, boost::is_any_of("\t\n\r "));
         if(parts.size() != 3)
            continue;

         if(parts[0] == "uniform")
         {
            auto iter = m_uniformStringLookups.find(parts[2]);
            if(iter != m_uniformStringLookups.end())
            {
               if(parts[1] == iter->second.typeName)
                  shader.uniforms.push_back(iter->second.ID);
            }
         }
         else if(parts[0] == "attribute")
         {
            auto iter = m_attributeStringLookups.find(parts[2]);
            if(iter != m_attributeStringLookups.end())
            {
               if(parts[1] == iter->second.typeName)
                  shader.attributes.push_back(iter->second.ID);
            }               
         }
      }

      file.close();
   }

   void parseShader(Shader &shader)
   {
      parseShaderFile(shader, shader.vertexPath);
      parseShaderFile(shader, shader.fragmentPath);
   }

   int compileShader(std::string path, int type)
   {
      auto st = IOC.resolve<StringTable>();
      auto shader = readFullFile(path);

      unsigned int handle = glCreateShader(type);
      if(handle)
      {
         int shaderLength = shader.length();
         const char *shaderArr = shader.c_str();

         glShaderSource(handle, 1, &shaderArr, &shaderLength);
         glCompileShader(handle);

         int compileStatus;
         glGetShaderiv(handle, GL_COMPILE_STATUS, &compileStatus);
         if(!compileStatus)
         {
            //int infoLen;
            //glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &infoLen);
            //glGetShaderInfoLog(handle, infoLen, 
            throw ArqException("Failed to compile shader");
         }

         return handle;
      }

      throw ArqException("Failed to compile shader");
   }

   int buildShaderProgram(int vertex, int fragment)
   {
      int handle = glCreateProgram();
      if(handle)
      {
         glAttachShader(handle, vertex);
         glAttachShader(handle, fragment);
         glLinkProgram(handle);

         int linkStatus;
         glGetProgramiv(handle, GL_LINK_STATUS, &linkStatus);
         if(!linkStatus)
            throw ArqException("Failed to create program.");

         return handle;
      }

      throw ArqException("Failed to create program.");
   }

   void clearUniformAndAttributeHandles()
   {
      std::fill(std::begin(m_uniforms), std::end(m_uniforms), -1);
      std::fill(std::begin(m_attributes), std::end(m_attributes), -1);
   }

   void buildShader(Shader &shader)
   {
      int vert = compileShader(shader.vertexPath, GL_VERTEX_SHADER);
      int frag = compileShader(shader.fragmentPath, GL_FRAGMENT_SHADER);

      shader.handle = buildShaderProgram(vert, frag);
   }
   std::atomic_bool m_built;
public:
   Impl()
      :m_currentShader(nullptr), m_currentView(IdentityMatrix())
   {
      m_built = false;
      buildLookups();
      clearUniformAndAttributeHandles();
   }
   bool built() {return m_built; }
   void buildShaders()
   {
      auto st = IOC.resolve<StringTable>();
      std::unordered_map<std::string, Shader> unbuiltShaders;

      auto shaders = IOC.resolve<AssetManager>()->getFiles("assets/shaders");

      std::vector<std::string> parts;

      //build up a map of unbuilt shaders with their necessary vertex and fragment paths
      for(auto &shader : shaders)
      {
         auto lastSlashIndex = shader.find_last_of('/');
         auto filename = shader.substr(lastSlashIndex + 1, shader.length() - lastSlashIndex - 1);
      
         boost::split(parts, filename, boost::is_any_of("."));
         if(parts.size() != 2)
            continue;

         Shader *currentShader;
         auto iter = unbuiltShaders.find(parts[0]);
         if(iter == unbuiltShaders.end())
         {
            auto newShader = unbuiltShaders.insert(std::make_pair(parts[0], Shader()));
            currentShader = &newShader.first->second;
         }
         else
            currentShader = &iter->second;

         if(parts[1] == "vert")
            currentShader->vertexPath = shader;
         else if(parts[1] == "frag")
            currentShader->fragmentPath = shader;      
      }

      //now loop through our map and build shaders who have both paths
      for(auto &pair : unbuiltShaders)
      {
         auto &shader = pair.second;
         if(!shader.vertexPath.empty() && !shader.fragmentPath.empty())
         {
            parseShader(shader);
            buildShader(shader);
            m_shaders.insert(std::make_pair(st->get(pair.first), shader));
         }
      }

      m_built = true;
   }

   void setAttribute(ShaderAttribute attr, int dataSize, int stride, int offset)
   {
      auto handle = m_attributes[(unsigned int)attr];
      if(handle == -1) return;
      
      glVertexAttribPointer(handle, dataSize / sizeof(float), GL_FLOAT, false, stride, (void*)offset);
      glEnableVertexAttribArray(handle);
   }

   void setUniform(ShaderUniform u, const float *data)
   {
      auto handle = m_uniforms[(unsigned int)u];
      if(handle == -1) return;

      UniformType type = m_uniformTypes[(unsigned int)u];
      switch(type)
      {
      case UniformType::mat4:
         glUniformMatrix4fv(handle, 1, false, data);
         break;
      case UniformType::vec4:
         glUniform4fv(handle, 1, data);
         break;
      }

      if(u == ShaderUniform::View)
         m_currentView = *(Matrix *)data;
      
   }

   void bindTextureUniform()
   {
      auto handle = m_uniforms[(unsigned int)ShaderUniform::Texture];
      if(handle == -1) return;
      glUniform1i(handle, 0);
   }

   void useShader(InternString name)
   {
      if(m_currentShader != name)
      {
         auto iter = m_shaders.find(name);
         if(iter != m_shaders.end())
         {
            clearUniformAndAttributeHandles();            

            auto &shader = iter->second;
            glUseProgram(shader.handle);

            for(auto u : shader.uniforms)
            {
               auto &uLookup = m_uniformEnumLookups[u];
               auto handle = glGetUniformLocation(shader.handle, uLookup.name.c_str());
               m_uniforms[(unsigned int)u] = handle;
            }

            for(auto attr : shader.attributes)
            {
               auto &aLookup = m_attributeEnumLookups[attr];
               auto handle = glGetAttribLocation(shader.handle, aLookup.name.c_str());
               m_attributes[(unsigned int)attr] = handle;
            } 

            m_currentShader = iter->first;
            setUniform(ShaderUniform::View, m_currentView.data());
         }
      }
   }
};

ShaderManager::ShaderManager()
   :pImpl(new Impl())
{
}

ShaderManager::~ShaderManager()
{
}

void ShaderManager::buildShaders(){pImpl->buildShaders();}
bool ShaderManager::built(){return pImpl->built();}

void ShaderManager::setAttribute(ShaderAttribute attr, int dataSize, int stride, int offset)
{
   pImpl->setAttribute(attr, dataSize, stride, offset);
}
void ShaderManager::setUniform(ShaderUniform u, const float *data)
{
   pImpl->setUniform(u, data);
}

void ShaderManager::useShader(InternString name){pImpl->useShader(name);}

void ShaderManager::bindTextureUniform(){pImpl->bindTextureUniform();}