#include "Dialog.h"
#include "engine\CoreUI.h"
#include "engine\UIDecorator.h"
#include "engine\UISubsystem.h"
#include "InputDefinitions.h"

static const float Padding = 8.0f;
static const float BorderWidth = 2.0f;
static const float HeaderHeight = 20.0f;
static const Colorf BackgroundColor(0.5f, 0.5f, 0.5f, 0.5f);
static const Colorf BorderColor(0.25f, 0.25f, 0.25f);

class Dialog::Impl
{
   std::unique_ptr<UIElement> m_dlg;
   UIElement *m_innerPanel;
   bool m_maximized;
   Rectf m_minSize;
public:
   Impl(std::string title, float x, float y, float width, float height)
      :m_maximized(false)
   {
      Rectf dlgBounds(x, y, x+width, y+height);      

      m_dlg = 
         buildDraggableElement(
         buildResizableSubsystem(
         buildBorderedSubsystem(
         buildBasicSubsystem(
         buildBackgroundedElement(
            CoreUI::buildBasicElement(dlgBounds, CoreUI::Layouts::buildFreeLayout()))))));
      m_dlg->setOption(UIOption::BackgroundColor, BackgroundColor);
      m_dlg->setOption(UIOption::BorderColor, BorderColor);
      m_dlg->setOption(UIOption::BackgroundType, UIBackgroundType::Gradient);
      m_dlg->setOptionAllBorders(BorderWidth);
      m_dlg->setOptionAllPadding(Padding);

      m_innerPanel = m_dlg->intern(
         buildBorderedSubsystem(
         buildBasicSubsystem(
         buildBackgroundedElement(
            CoreUI::buildBasicElement(Rectf(), CoreUI::Layouts::buildFreeLayout())))));
      m_innerPanel->anchorToParent();
      m_innerPanel->setOption(UIOption::Top, HeaderHeight + Padding);
      m_innerPanel->setOption(UIOption::BackgroundColor, Colorf(0.1f, 0.1f, 0.1f));
      m_innerPanel->setOption(UIOption::BorderColor, BorderColor);
      m_innerPanel->setOptionAllBorders(BorderWidth);

      auto maxButton = m_dlg->intern(
         buildBackgroundedElement(
         CoreUI::buildBasicElement(Rectf(), CoreUI::Layouts::buildFreeLayout())));
      maxButton->setOption(UIOption::BackgroundColor, Colorf(1.0f, 1.0f, 1.0f));
      maxButton->setOption(UIOption::Right, 0.0f);
      maxButton->setOption(UIOption::Top, 0.0f);
      maxButton->setOption(UIOption::Height, HeaderHeight);
      maxButton->setOption(UIOption::Width, HeaderHeight);
      maxButton->registerMouseButton(Input::MouseLeft, Input::Press, 0, [&](MouseEvent e){});
      maxButton->registerMouseButton(Input::MouseLeft, Input::Release, 0, [&](MouseEvent e)
      {
         if(m_maximized)
         {
            m_dlg->clearAnchors();
            m_dlg->getBounds().absolute = m_minSize;
            m_maximized = false;
            
         }
         else
         {
            m_minSize = m_dlg->getBounds().absolute;
            m_dlg->anchorToParent();
            m_maximized = true;
         }

         m_dlg->getParent()->arrange();
      });

      auto titleFont = std::make_shared<TextFont>("assets/fonts/pressstart.ttf", 12.0f);
      auto titleElement = m_dlg->intern(CoreUI::buildTextElement(title, titleFont, Float2(0, HeaderHeight)));

      m_dlg->pushListChild(m_innerPanel);
      m_dlg->pushListChild(maxButton);
      m_dlg->pushListChild(titleElement);
   }

   ~Impl(){}
   void open(UIElement *parent){parent->pushListChild(m_dlg.get());}
   void close()
   {
      if(auto p = m_dlg->getParent())
         if(auto l = p->getLayout())
         {
            l->removeChild(m_dlg.get());
            m_dlg->setParent(nullptr);
         }
            
   }
   void setMaximized(bool set)
   {
      if(m_maximized)
      {
         if(!set)
         {
            m_dlg->clearAnchors();
            m_dlg->getBounds().absolute = m_minSize;
            m_maximized = false; 
         }       
      }
      else if(set)
      {
         m_minSize = m_dlg->getBounds().absolute;
         m_dlg->anchorToParent();
         m_maximized = true;
      }

      m_dlg->getParent()->arrange();
   }
   UIElement *element(){return m_innerPanel;}
   UIElement *outerElement(){return m_dlg.get();}
};


Dialog::Dialog(std::string title, float x, float y, float width, float height)
   :pImpl(new Impl(title, x, y, width, height)){}
Dialog::~Dialog(){}
void Dialog::open(UIElement *parent){pImpl->open(parent);}
void Dialog::close(){pImpl->close();}
UIElement *Dialog::element(){return pImpl->element();}
UIElement *Dialog::outerElement(){return pImpl->outerElement();}
void Dialog::setMaximized(bool maximized){pImpl->setMaximized(maximized);}
