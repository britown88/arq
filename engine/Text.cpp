#include "Text.h"
#include <GLFW/glfw3.h>

Font::Font(TextFont font, FontEngine* fontEngine) : m_font(font), m_fontEngine(fontEngine)
{
   buildFont();
}
const FontChar *Font::getCharacters() const{return m_chars;}

FontEngine::FontEngine()
{
   if (FT_Init_FreeType( &m_library )) 
      throw std::runtime_error("FT_Init_FreeType failed");
}

FontEngine::~FontEngine()
{
   FT_Done_FreeType(m_library);
}

Font& FontEngine::getFont(TextFont const& font)
{
   auto iter = m_fontMap.find(font);
   if (iter != m_fontMap.end())
   {
      return *iter->second;
   }
   m_fontMap.insert(std::make_pair(font, std::unique_ptr<Font>(new Font(font, this))));
   return getFont(font);
}

FT_Library FontEngine::GetLibrary()
{
   return m_library;
}

// This Function Gets The First Power Of 2 >= The
// Int That We Pass It.
inline int next_p2 (int a )
{
   int rval=1;
   // rval<<=1 Is A Prettier Way Of Writing rval*=2;
   while(rval<a) rval<<=1;
   return rval;
}
void buildDisplayList(FT_Face face, char ch, FontChar &fontChar)
{
   // The First Thing We Do Is Get FreeType To Render Our Character
   // Into A Bitmap.  This Actually Requires A Couple Of FreeType Commands:

   // Load The Glyph For Our Character.
   if(FT_Load_Glyph( face, FT_Get_Char_Index( face, ch ), FT_LOAD_DEFAULT ))
      throw std::runtime_error("FT_Load_Glyph failed");

   // Move The Face's Glyph Into A Glyph Object.
   FT_Glyph glyph;
   if(FT_Get_Glyph( face->glyph, &glyph ))
      throw std::runtime_error("FT_Get_Glyph failed");

   // Convert The Glyph To A Bitmap.
   FT_Glyph_To_Bitmap( &glyph, ft_render_mode_normal, 0, 1 );
   FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;

   // This Reference Will Make Accessing The Bitmap Easier.
   FT_Bitmap& bitmap = bitmap_glyph->bitmap;

   // Use Our Helper Function To Get The Widths Of
   // The Bitmap Data That We Will Need In Order To Create
   // Our Texture.
   int width = next_p2( bitmap.width );
   int height = next_p2( bitmap.rows );

   // Allocate Memory For The Texture Data.
   GLubyte* expanded_data = new GLubyte[ 2 * width * height];

   // Here We Fill In The Data For The Expanded Bitmap.
   // Notice That We Are Using A Two Channel Bitmap (One For
   // Channel Luminosity And One For Alpha), But We Assign
   // Both Luminosity And Alpha To The Value That We
   // Find In The FreeType Bitmap.
   // We Use The ?: Operator To Say That Value Which We Use
   // Will Be 0 If We Are In The Padding Zone, And Whatever
   // Is The FreeType Bitmap Otherwise.
   for(int j=0; j <height;j++) {
      for(int i=0; i < width; i++){
         expanded_data[2*(i+j*width)]= expanded_data[2*(i+j*width)+1] =
            (i>=bitmap.width || j>=bitmap.rows) ?
            0 : bitmap.buffer[i + bitmap.width*j];
      }
   }

   // Now We Just Setup Some Texture Parameters.
   glBindTexture( GL_TEXTURE_2D, fontChar.texture);
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

   // Here We Actually Create The Texture Itself, Notice
   // That We Are Using GL_LUMINANCE_ALPHA To Indicate That
   // We Are Using 2 Channel Data.
   glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
      GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, expanded_data );

   // With The Texture Created, We Don't Need The Expanded Data Anymore.
   delete [] expanded_data;

   glBindTexture(GL_TEXTURE_2D, 0);

   auto &tc = fontChar.texCoord;
   auto &m = fontChar.model;

   MatrixTransforms::translate(m, 
      (float)bitmap_glyph->left, 
      glyph->advance.y - (float)bitmap_glyph->top);

   //tex coord scaling
   float   dx= (float)bitmap.width / (float)width,
           dy= (float)bitmap.rows / (float)height;

   MatrixTransforms::scale(tc, dx, dy);
   //MatrixTransforms::scale(m, bitmap.width, bitmap.rows);

   fontChar.scaleX = bitmap.width;
   fontChar.scaleY = bitmap.rows;

   fontChar.width = (float)(face->glyph->advance.x >> 6);
   fontChar.height = (float)bitmap_glyph->top - glyph->advance.y;
}

float Font::getHeight()
{
   return m_maxHeight;
}

void Font::buildFont()
{

   // The Object In Which FreeType Holds Information On A Given
   // Font Is Called A "face".
   FT_Face face;

   // This Is Where We Load In The Font Information From The File.
   // Of All The Places Where The Code Might Die, This Is The Most Likely,
   // As FT_New_Face Will Fail If The Font File Does Not Exist Or Is Somehow Broken.
   if (FT_New_Face( m_fontEngine->GetLibrary(), m_font.fontFile.c_str(), 0, &face ))
      throw std::runtime_error("FT_New_Face failed (there is probably a problem with your font file)");

   // For Some Twisted Reason, FreeType Measures Font Size
   // In Terms Of 1/64ths Of Pixels.
   int height = m_font.height * 64;

   FT_Set_Char_Size( face, height, height, 96, 96);

   // Here We Ask OpenGL To Allocate Resources For
   // All The Textures And Display Lists Which We
   // Are About To Create. 
   GLuint textures[128];
   glGenTextures( 128, &textures[0] );

   // This Is Where We Actually Create Each Of The Fonts Display Lists.
   for(unsigned char i=0;i<128;i++)
   {
      m_chars[i].texture = textures[i];
      buildDisplayList(face, i, m_chars[i]);
   }

   m_maxHeight = 0.0f;
   for(unsigned char i=0;i<128;i++)
      m_maxHeight = std::max(m_chars[i].height, m_maxHeight);

   for(unsigned char i=0;i<128;i++)
   {
      //MatrixTransforms::translate(m_chars[i].model, 0.0f, m_maxHeight / 2.0f);
      MatrixTransforms::scale(m_chars[i].model, m_chars[i].scaleX, m_chars[i].scaleY);
   }

   // We Don't Need The Face Information Now That The Display
   // Lists Have Been Created, So We Free The Associated Resources.
   FT_Done_Face(face);
}
