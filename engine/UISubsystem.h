#pragma once

#include "UIDecorator.h"
#include "CoreUI.h"

//what follows here is bullshit

class SubsystemElement : public UIElement
{
protected:
   std::unique_ptr<UIElement> m_inner;
   std::unique_ptr<UIElement> m_innerContainer;
   std::unique_ptr<UILayout> m_layout;
   UIElementBounds m_bounds;

   std::unique_ptr<SubsystemElement> m_innerSystem;

public:
   SubsystemElement(std::unique_ptr<SubsystemElement> inner)
      :m_innerSystem(std::move(inner))
   {
   }

   SubsystemElement(std::unique_ptr<UIElement> inner)
      :m_inner(std::move(inner))
   {
      m_inner->setHighestDecorator(this);
      UIElement::setHighestDecorator(this);

      m_bounds = m_inner->getBounds();
      m_layout = std::move(CoreUI::Layouts::buildFreeLayout());
      m_layout->owner = this;

      m_innerContainer = CoreUI::buildBasicElement(Rectf(), CoreUI::Layouts::buildFreeLayout());
      m_innerContainer->anchorToParent();

      UIElement::pushListChild(m_inner.get());
      m_inner->anchorToParent();

      UIElement::pushListChild(m_innerContainer.get());  
   }

   virtual UIElement *getContainer(){return m_innerContainer.get();}

   //uielement shit
   virtual void setParent(UIElement *p)
   {
      for(auto child : m_layout->getChildren())
         child->setInputParent(p);
         //child->inputParent = p;
         //child->setParent(p);

     m_inner->setParent(p);
     m_innerContainer->setParent(p);
     UIElement::setParent(p);

   }
   virtual void setInputParent(UIElement *e){m_inner->setInputParent(e);}
   virtual void setHighestDecorator(UIElement *e)
   {
      m_inner->setHighestDecorator(e);
      UIElement::setHighestDecorator(e);
   }
   virtual UIElement *getParent()
   {
      return m_inner->getParent();
   }
   virtual UILayout *getLayout(){return m_layout.get();}
   virtual void draw(Renderer &r)
   {
      r.pushScissor(m_bounds.outerClipped);

      for(auto child : m_layout->getChildren())
         child->draw(r);

      r.popScissor();
   } 
   virtual void onStep(){UIElement::onStep();} 
   virtual UIElementBounds &getBounds()
   {
      return m_bounds;
   }
   virtual void update(){UIElement::update();}
   virtual void arrange(){UIElement::arrange();}

   //ok so we need to filter certain options settings to redirect to different places =/
   //self needs to hold the margins and innercontainer needs to hold the padding
   virtual boost::optional<UIOptionVar> getOption(UIOption opt){return UIElement::getOption(opt);}
   virtual boost::optional<UIOptionVar> getInnerOption(UIOption opt){return m_inner->getOption(opt);}
   virtual void setOption(UIOption opt, UIOptionVar var)
   {
      switch(opt)
      {
      case UIOption::Height:
      case UIOption::Width:
      case UIOption::Left:
      case UIOption::Top:
      case UIOption::Right:
      case UIOption::Bottom:
      case UIOption::MarginLeft:
      case UIOption::MarginTop:
      case UIOption::MarginRight:
      case UIOption::MarginBottom:
         UIElement::setOption(opt, var);
         break;
      case UIOption::PaddingLeft:
      case UIOption::PaddingTop:
      case UIOption::PaddingRight:
      case UIOption::PaddingBottom:
         m_innerContainer->setOption(opt, var);
         break;
      default:
         m_inner->setOption(opt, var);
         break;
      };
   }
   virtual void clearOption(UIOption opt)
   {
      switch(opt)
      {
      case UIOption::Height:
      case UIOption::Width:
      case UIOption::Left:
      case UIOption::Top:
      case UIOption::Right:
      case UIOption::Bottom:
      case UIOption::MarginLeft:
      case UIOption::MarginTop:
      case UIOption::MarginRight:
      case UIOption::MarginBottom:
         UIElement::clearOption(opt);
         break;
      case UIOption::PaddingLeft:
      case UIOption::PaddingTop:
      case UIOption::PaddingRight:
      case UIOption::PaddingBottom:
         m_innerContainer->clearOption(opt);
         break;
      default:
         m_inner->clearOption(opt);
         break;
      };
   }
   virtual void setOptionAllMargins(float width){UIElement::setOptionAllMargins(width);}
   virtual void setOptionAllPadding(float width){m_innerContainer->setOptionAllPadding(width);}

   //redirect all children pushing/popping to the innercontainer
   virtual void pushGridChild(UIElement *elem, size_t column){m_innerContainer->pushGridChild(elem, column);}
   virtual void popGridChild(size_t column){m_innerContainer->popGridChild(column);}
   virtual void insertGridChild(UIElement *elem, size_t index, size_t column){m_innerContainer->insertGridChild(elem, index, column);}
   virtual void eraseGridChild(size_t index, size_t column){m_innerContainer->eraseGridChild(index, column);}
   virtual void pushListChild(UIElement *elem){m_innerContainer->pushListChild(elem);}
   virtual void popListChild(){m_innerContainer->popListChild();}
   virtual void insertListChild(UIElement *elem, size_t index){m_innerContainer->insertListChild(elem, index);}
   virtual void eraseListChild(size_t index){m_innerContainer->eraseListChild(index);}

   virtual void registerMouseCallback(MouseEvent e, UIMouseCallback cb){m_inner->registerMouseCallback(e, cb);}
   virtual void unRegisterMouseCallback(MouseEvent e){m_inner->unRegisterMouseCallback(e);}
   virtual bool onMouseEvent(MouseEvent e){return m_inner->onMouseEvent(e);}
   virtual void giveMouseFocus(MouseHandler *handler){m_inner->giveMouseFocus(handler);}
   virtual void removeMouseFocus(){m_inner->removeMouseFocus();}

   //UI Helpers
   virtual void registerMouseButton(int button, int action, int mods, UIMouseCallback cb){m_inner->registerMouseButton(button, action, mods, cb);}
   virtual void registerMouseMove(UIMouseCallback cb){m_inner->registerMouseMove(cb);}
   virtual void registerMouseScroll(int mods, UIMouseCallback cb){m_inner->registerMouseScroll(mods, cb);}
   virtual void registerMouseEnter(bool entered, UIMouseCallback cb){m_inner->registerMouseEnter(entered, cb);}
   virtual void unregisterMouseButton(int button, int action, int mods){m_inner->unregisterMouseButton(button, action, mods);}
   virtual void unregisterMouseMove(){m_inner->unregisterMouseMove();}
   virtual void unregisterMouseScroll(int mods){m_inner->unregisterMouseScroll(mods);}
   virtual void unregisterMouseEnter(bool entered){m_inner->unregisterMouseEnter(entered);}

   //helpers
   virtual void setOptionAllBorders(float width){m_inner->setOptionAllBorders(width);}
   virtual void anchorToParentWidth(float distance = 0.0f){UIElement::anchorToParentWidth(distance);}
   virtual void anchorToParentHeight(float distance = 0.0f){UIElement::anchorToParentHeight(distance);}
   virtual void anchorToParent(float distance = 0.0f){UIElement::anchorToParent(distance);}
   virtual void setOptionAnchors(float left, float top, float right, float bottom){UIElement::setOptionAnchors(left, top, right, bottom);}
   virtual void clearAnchors(){UIElement::clearAnchors();}
};

class SubsystemDecorator : public SubsystemElement
{
public:
   SubsystemDecorator(std::unique_ptr<SubsystemElement> system)
      :SubsystemElement(std::move(system)){m_innerSystem->setHighestDecorator(this);}

   virtual UIElement *getContainer(){return m_innerSystem->getContainer();}
   virtual void setParent(UIElement *p){m_innerSystem->setParent(p);}
   virtual UIElement *getParent(){return m_innerSystem->getParent();}
   virtual void setHighestDecorator(UIElement *e){m_innerSystem->setHighestDecorator(e);}
   virtual void setInputParent(UIElement *e){m_innerSystem->setInputParent(e);}

   virtual UILayout *getLayout(){return m_innerSystem->getLayout();}
   virtual void draw(Renderer &r){m_innerSystem->draw(r);}
   virtual void onStep(){m_innerSystem->onStep();} 
   virtual UIElementBounds &getBounds(){return m_innerSystem->getBounds();}
   virtual void update(){m_innerSystem->update();}
   virtual void arrange(){m_innerSystem->arrange();}

   virtual boost::optional<UIOptionVar> getOption(UIOption opt){return m_innerSystem->getOption(opt);}
   virtual boost::optional<UIOptionVar> getInnerOption(UIOption opt){return m_innerSystem->getInnerOption(opt);}
   virtual void setOption(UIOption opt, UIOptionVar var){m_innerSystem->setOption(opt, var);}
   virtual void clearOption(UIOption opt){m_innerSystem->clearOption(opt);}
   virtual void setOptionAllMargins(float width){m_innerSystem->setOptionAllMargins(width);}
   virtual void setOptionAllPadding(float width){m_innerSystem->setOptionAllPadding(width);}

   virtual void pushGridChild(UIElement *elem, size_t column){m_innerSystem->pushGridChild(elem, column);}
   virtual void popGridChild(size_t column){m_innerSystem->popGridChild(column);}
   virtual void insertGridChild(UIElement *elem, size_t index, size_t column){m_innerSystem->insertGridChild(elem, index, column);}
   virtual void eraseGridChild(size_t index, size_t column){m_innerSystem->eraseGridChild(index, column);}
   virtual void pushListChild(UIElement *elem){m_innerSystem->pushListChild(elem);}
   virtual void popListChild(){m_innerSystem->popListChild();}
   virtual void insertListChild(UIElement *elem, size_t index){m_innerSystem->insertListChild(elem, index);}
   virtual void eraseListChild(size_t index){m_innerSystem->eraseListChild(index);}
 
   virtual void registerMouseCallback(MouseEvent e, UIMouseCallback cb){m_innerSystem->registerMouseCallback(e, cb);}
   virtual void unRegisterMouseCallback(MouseEvent e){m_innerSystem->unRegisterMouseCallback(e);}
   virtual bool onMouseEvent(MouseEvent e){return m_innerSystem->onMouseEvent(e);}
   virtual void giveMouseFocus(MouseHandler *handler){m_innerSystem->giveMouseFocus(handler);}
   virtual void removeMouseFocus(){m_innerSystem->removeMouseFocus();}

   //UI Helpers
   virtual void registerMouseButton(int button, int action, int mods, UIMouseCallback cb){m_innerSystem->registerMouseButton(button, action, mods, cb);}
   virtual void registerMouseMove(UIMouseCallback cb){m_innerSystem->registerMouseMove(cb);}
   virtual void registerMouseScroll(int mods, UIMouseCallback cb){m_innerSystem->registerMouseScroll(mods, cb);}
   virtual void registerMouseEnter(bool entered, UIMouseCallback cb){m_innerSystem->registerMouseEnter(entered, cb);}
   virtual void unregisterMouseButton(int button, int action, int mods){m_innerSystem->unregisterMouseButton(button, action, mods);}
   virtual void unregisterMouseMove(){m_innerSystem->unregisterMouseMove();}
   virtual void unregisterMouseScroll(int mods){m_innerSystem->unregisterMouseScroll(mods);}
   virtual void unregisterMouseEnter(bool entered){m_innerSystem->unregisterMouseEnter(entered);}

   //helpers
   virtual void setOptionAllBorders(float width){m_innerSystem->setOptionAllBorders(width);}
   virtual void anchorToParentWidth(float distance = 0.0f){m_innerSystem->anchorToParentWidth(distance);}
   virtual void anchorToParentHeight(float distance = 0.0f){m_innerSystem->anchorToParentHeight(distance);}
   virtual void anchorToParent(float distance = 0.0f){m_innerSystem->anchorToParent(distance);}
   virtual void setOptionAnchors(float left, float top, float right, float bottom){m_innerSystem->setOptionAnchors(left, top, right, bottom);}
   virtual void clearAnchors(){m_innerSystem->clearAnchors();}
};

std::unique_ptr<SubsystemElement> buildBasicSubsystem(std::unique_ptr<UIElement> inner);
std::unique_ptr<SubsystemElement> buildBorderedSubsystem(std::unique_ptr<SubsystemElement> inner);
std::unique_ptr<SubsystemElement> buildResizableSubsystem(std::unique_ptr<SubsystemElement> inner);