#include "UIDecorator.h"
#include "GameHelpers.h"
#include "TextureManager.h"
#include "IOCContainer.h"

class BackgroundedElement : public DecoratorUIElement
{
   Colorf m_color;
   IBOPtr m_ibo;
   VBOPtr m_vbo;
   
   UIBackgroundType m_bgType;
   Texture *m_texture;
   Matrix m_texTransform;
public:
   BackgroundedElement(std::unique_ptr<UIElement> inner)
      :DecoratorUIElement(std::move(inner)), m_bgType(UIBackgroundType::Solid), 
      m_texture(nullptr), m_texTransform(IdentityMatrix())
   {
      m_vbo = GameHelpers::standardRectangleVBO();
      m_ibo = GameHelpers::standardRectangleIBO();
      m_color = Colorf(1, 1, 1);
   }

   void updateColors()
   {
      if(auto c = getOption(UIOption::BackgroundColor))
         m_color = boost::get<Colorf>(*c);

      if(auto bg = getOption(UIOption::BackgroundType))
      {
         auto bgType = boost::get<UIBackgroundType>(*bg);
         if(bgType != m_bgType)
         {
            m_bgType = bgType;
            switch(m_bgType)
            {
            case UIBackgroundType::Solid:
               m_vbo = GameHelpers::standardRectangleVBO();
               break;
            case UIBackgroundType::Gradient:
               m_vbo = GameHelpers::gradientRectangleVBO();
               break;
            case UIBackgroundType::Image:
               m_vbo = GameHelpers::standardRectangleVBO();
               auto texName = boost::get<InternString>(*getOption(UIOption::BackgroundImage));
               m_texture = IOC.resolve<TextureManager>()->getTexture(TextureRequest(texName));
               break;
            };
         }         
      }
   }

   void draw(Renderer &r)
   {
      updateColors();
      Matrix m;
      MatrixTransforms::toRect(m, getBounds().relative);

      r.pushScissor(getBounds().outerClipped);
      if(m_texture)
         r.drawTexture(m_vbo, m_ibo, m_texture, m, m_texTransform, m_color);
      else
         r.drawTriangles(m_vbo, m_ibo, m, m_color);

      m_inner->draw(r);

      r.popScissor();
   } 
};

std::unique_ptr<UIElement> buildBackgroundedElement(std::unique_ptr<UIElement> inner)
{
   return std::unique_ptr<BackgroundedElement>(new BackgroundedElement(std::move(inner)));
}