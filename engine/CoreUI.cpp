#include "CoreUI.h"
#include "IOCContainer.h"
#include "Application.h"
#include "RenderManager.h"
#include "Matrix.h"
#include "ComponentHelpers.h"
#include "InputDefinitions.h"
#include "UIDecorator.h"

class RootUIElement : public UIElement
{
   std::unique_ptr<UILayout> m_layout;
   UIElementBounds m_bounds;
public:
   RootUIElement()
   {
      m_layout = std::move(CoreUI::Layouts::buildFreeLayout());
      m_layout->owner = this;

      auto app = IOC.resolve<Application>();
      auto winSize = app->windowSize();

      m_bounds.absolute = m_bounds.relative = 
         m_bounds.outerClipped = m_bounds.innerClipped = Rectf(0, 0, winSize.x, winSize.y);
   }

   void draw(Renderer &r)
   {
      r.pushViewport(m_bounds.absolute);
      r.pushCamera(m_bounds.absolute);

      for(auto child : m_layout->getChildren())
         child->draw(r);

      r.popCamera();
      r.popViewport();
      r.swapBuffers();
   }

   UILayout *getLayout(){return m_layout.get();}
   UIElementBounds &getBounds(){return m_bounds;}
};

std::unique_ptr<UIElement> CoreUI::buildRootUIElement()
{
   return std::unique_ptr<UIElement>(new RootUIElement());
}


class WorldUIElementImpl : public CoreUI::WorldUIElement
{
   std::unique_ptr<UILayout> m_layout;
   Rectf m_cameraBounds;
   RenderManager *m_renderManager;

   bool m_dragging;
   Float2 m_clickPoint;
   Rectf startingBounds;
   UIElementBounds m_bounds;
public:
   WorldUIElementImpl(const Rectf &viewport, RenderManager *rm)
   {
      m_layout = std::move(CoreUI::Layouts::buildFreeLayout());
      m_layout->owner = this;

      m_bounds.absolute = viewport;
      m_renderManager = rm;

      registerMouseScroll(0, [&](MouseEvent e){onScroll(e.scrollX, e.scrollY);});
      registerMouseButton(Input::MouseLeft, Input::Press, 0, [&](MouseEvent e){onMouseDown(e.x, e.y);});
      registerMouseButton(Input::MouseLeft, Input::Release, 0, [&](MouseEvent e){onMouseUp(e.x, e.y);});
      registerMouseMove([&](MouseEvent e){onMouseMove(e.x, e.y);});

      m_dragging = false;
   }

   void onStep()
   {
   }

   void onMouseMove(float x, float y)
   {
      if(m_dragging)
      {
         m_cameraBounds = startingBounds;
         m_cameraBounds.offset(Float2(-(x - m_clickPoint.x), -(y - m_clickPoint.y)));
      }
   }

   void onMouseDown(float x, float y)
   {
      m_dragging = true;
      m_clickPoint = Float2(x, y);
      startingBounds = m_cameraBounds;
   }

   void onMouseUp(float x, float y)
   {
      m_dragging = false;
   }

   void onScroll(float x, float y)
   {
      float sc = y * 100.0f;
      auto w = m_cameraBounds.width();
      auto h = m_cameraBounds.height();

      float r = std::min((w + sc) / w, (h + sc) / h);

      auto dw = w * r - w;
      auto dh = h * r - h;

      if(w - dw > 0 && h - dh > 0)
      {
         m_cameraBounds.inset(Float2(dw/2, dh/2));
      }
   }

   Rectf buildProportionalViewport()
   {
      Rectf rel = m_bounds.relative;
      float rw = rel.width();
      float rh = rel.height();
      float cw = m_cameraBounds.width();
      float ch = m_cameraBounds.height();
      float ratio = std::min(rw/cw, rh/ch);

      Rectf vp(0.0f, 0.0f, cw * ratio, ch * ratio);
      vp.offset(Float2((rw - vp.width()) / 2.0f, (rh - vp.height()) / 2.0f));
      vp.offset(Float2(rel.left, rel.top));

      return vp;
   }

   void draw(Renderer &r)
   {
      r.pushViewport(buildProportionalViewport());
      //r.pushViewport(m_bounds.relative);
      r.pushCamera(m_cameraBounds);

      r.pushScissor(m_bounds.outerClipped);

      m_renderManager->draw(r);

      r.popCamera();


      for(auto child : m_layout->getChildren())
         child->draw(r);

      r.popScissor();
      
      r.popViewport();
   }

   UILayout *getLayout(){return m_layout.get();}

   Rectf &cameraBounds(){return m_cameraBounds;}
   UIElementBounds &getBounds(){return m_bounds;}
};

std::unique_ptr<CoreUI::WorldUIElement> CoreUI::buildWorldUIElement(const Rectf &viewport, RenderManager *rm)
{
   return std::unique_ptr<WorldUIElement>(new WorldUIElementImpl(viewport, rm));
}

class TextElement : public UIElement
{
   UIElementBounds m_bounds;
   TextString m_string;

public:
   TextElement(std::string text, std::shared_ptr<TextFont> font, Float2 position)
   {
      m_string.text = text;
      m_string.font = font;
      m_bounds.absolute.offset(position);
   }

   void draw(Renderer &r)
   {
      Matrix m = IdentityMatrix();
      MatrixTransforms::translate(m, m_bounds.outerClipped.left, m_bounds.outerClipped.top);

      r.drawText(m_string, m, Colorf(1, 1, 1));
   }

   UILayout *getLayout(){return nullptr;}
   UIElementBounds &getBounds(){return m_bounds;}
};



std::unique_ptr<UIElement> CoreUI::buildTextElement(std::string text, std::shared_ptr<TextFont> font, Float2 position)
{
   return std::unique_ptr<TextElement>(new TextElement(text, font, position));
}

class BasicElement : public UIElement
{
protected:
   std::unique_ptr<UILayout> m_layout;
   UIElementBounds m_bounds;

public:
   BasicElement(const Rectf &rect, std::unique_ptr<UILayout> layout)
      :m_layout(std::move(layout))
   {
      m_layout->owner = this;
      m_bounds.absolute = rect;
   }

   void draw(Renderer &r)
   {
      r.pushScissor(m_bounds.innerClipped);

      for(auto child : m_layout->getChildren())
         child->draw(r);

      r.popScissor();
   }

   UILayout *getLayout(){return m_layout.get();}
   UIElementBounds &getBounds(){return m_bounds;}
};

std::unique_ptr<UIElement> CoreUI::buildBasicElement(const Rectf &bounds, std::unique_ptr<UILayout> layout)
{
   return std::unique_ptr<BasicElement>(new BasicElement(bounds, std::move(layout)));
}








