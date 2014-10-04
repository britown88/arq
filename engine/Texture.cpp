#include <GL\glew.h>

#include "Texture.h"
#include "IOCContainer.h"
#include "TextureManager.h"

#include "libpng\png.h"
#include "Color.h"

struct TextureBuffer
{
   std::unique_ptr<Color255[]> bits;
   Int2 size;

   TextureBuffer(std::unique_ptr<Color255[]> bits, Int2 size)
      :bits(std::move(bits)), size(size){}

   TextureBuffer(TextureBuffer && rhs)
      :bits(std::move(rhs.bits)), size(rhs.size){}
};

std::unique_ptr<TextureBuffer> loadPng(std::string const& textureFile)
{
   FILE* infile = fopen(textureFile.c_str(), "rb"); 
   if (!infile) {
      throw std::exception("failed to load texture.");
   }

   unsigned char sig[8];
   fread(sig, 1, 8, infile);
   if (!png_check_sig(sig, 8)) {
      fclose(infile);
      throw std::exception("failed to load texture.");
   }

   png_structp png_ptr; 
   png_infop info_ptr; 
   png_infop end_ptr;

   png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
   if (!png_ptr) {
      fclose(infile);
      throw std::exception("failed to load texture.");
   }

   info_ptr = png_create_info_struct(png_ptr);
   if (!info_ptr) {
      png_destroy_read_struct(&png_ptr, (png_infopp) NULL, (png_infopp) NULL);
      fclose(infile);
      throw std::exception("failed to load texture.");
   }

   end_ptr = png_create_info_struct(png_ptr);
   if (!end_ptr) {
      png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
      fclose(infile);
      throw std::exception("failed to load texture.");
   }

   if (setjmp(png_jmpbuf(png_ptr))) {
      png_destroy_read_struct(&png_ptr, &info_ptr, &end_ptr);
      fclose(infile);
      throw std::exception("failed to load texture.");
   }

   png_ptr->io_ptr = (png_voidp)infile;
   png_set_sig_bytes(png_ptr, 8);    


   int  bit_depth;
   int  color_type;

   unsigned long width;
   unsigned long height;
   unsigned int rowbytes;

   png_read_info(png_ptr, info_ptr);
   png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, 
      &color_type, NULL, NULL, NULL);

   png_read_update_info(png_ptr, info_ptr);

   if (bit_depth > 8) {
      png_set_strip_16(png_ptr);
   }
   if (color_type == PNG_COLOR_TYPE_GRAY ||
      color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
         png_set_gray_to_rgb(png_ptr);
   }
   if (color_type == PNG_COLOR_TYPE_PALETTE) {
      png_set_palette_to_rgb(png_ptr);
   }
   rowbytes = png_get_rowbytes(png_ptr, info_ptr);

   Color255* image_data;
   png_bytepp row_pointers = NULL;

   size_t totalSize = rowbytes*height;
   bool noAlpha = (rowbytes / width) == 3;
   if (noAlpha)
   {
      totalSize = (totalSize * 4) / 3;
   }

   if ((image_data =  new Color255[totalSize/4])==NULL) {
      png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
      throw std::exception("failed to load texture.");
   }

   if ((row_pointers = (png_bytepp)malloc(height*sizeof(png_bytep))) == NULL) {
      png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
      free(image_data);
      image_data = NULL;
      throw std::exception("failed to load texture.");
   }

   for (unsigned int i = 0;  i < height;  ++i)
      row_pointers[i] = (unsigned char*)(image_data) + i*(rowbytes);    

   png_read_image(png_ptr, row_pointers);

   free(row_pointers);
   png_destroy_read_struct(&png_ptr, &info_ptr, &end_ptr);
   fclose(infile);

   if (noAlpha)
   {
      unsigned char* actual = ((unsigned char*)image_data) + totalSize-1;
      unsigned char* original = ((unsigned char*)image_data) + ((totalSize/4) * 3)-1;
      for (unsigned int i = 0; i < width * height; ++i)
      {
         *actual-- = 255;
         *actual-- = *original--;
         *actual-- = *original--;
         *actual-- = *original--;
      }
   }

   return std::unique_ptr<TextureBuffer>(new TextureBuffer(std::unique_ptr<Color255[]>(image_data), Int2((int)width, (int)height)));
}


class Texture::Impl
{
   bool m_isLoaded;
   const TextureRequest m_request;
   unsigned int m_glHandle;
   Texture *m_parent;

   std::unique_ptr<TextureBuffer> m_buffer;
public:
   Impl(Texture *t, const TextureRequest &request)
      :m_request(request), m_isLoaded(false),
      m_glHandle(0), m_parent(t){}
   ~Impl(){}

   Int2 getSize()
   {
      return m_buffer ? m_buffer->size : Int2();
   }

   unsigned int getGLHandle()
   {
      //the load is carried about by the TM which calls acuire on this
      static TextureManager &tm = *IOC.resolve<TextureManager>();
      tm.acquire(m_parent);

      return m_glHandle;
   }

   void acquire()
   {
      if(!m_request.path)
         return;

      m_buffer = loadPng(*m_request.path);

      glEnable(GL_TEXTURE_2D);
      glGenTextures(1, &m_glHandle);
      glBindTexture(GL_TEXTURE_2D, m_glHandle);
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

      switch(m_request.filterType)
      {
      case FilterType::Linear:
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
         break;
      case FilterType::Nearest:
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
         break;
      };

      switch(m_request.repeatType)
      {
      case RepeatType::Repeat:
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
         break;
      case RepeatType::Clamp:
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
         break;
      };
      
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_buffer->size.x, m_buffer->size.y, 0,  GL_RGBA, GL_UNSIGNED_BYTE, m_buffer->bits.get());

      glBindTexture(GL_TEXTURE_2D, 0);

      m_isLoaded = true;
   }

   void release()
   {
      glDeleteTextures(1, &m_glHandle);
      m_buffer.reset();

      m_glHandle = 0;
      m_isLoaded = false;
   }

   bool isLoaded(){return m_isLoaded;}
};


Texture::Texture(const TextureRequest &request):pImpl(new Impl(this, request)){}
Texture::~Texture(){}

Int2 Texture::getSize(){return pImpl->getSize();}
unsigned int Texture::getGLHandle(){return pImpl->getGLHandle();}

void Texture::acquire(){pImpl->acquire();}
void Texture::release(){pImpl->release();}

bool Texture::isLoaded(){return pImpl->isLoaded();}