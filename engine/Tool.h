#pragma once

#include <string>
#include <memory>

#include "Vector.h"
#include "Dialog.h"

class UIElement;

class Tool
{
   std::unique_ptr<Dialog> m_dlg;
protected:
   virtual std::string getTitle(){return "";}
   virtual Float2 getStartingPosition(){return Float2();}
   virtual Float2 getStartingSize(){return Float2(100.0f, 100.0f);}

   virtual void onOpen(){}
   virtual void onStep(){}
   virtual void onClose(){}

public:
   Tool(){}
   virtual ~Tool(){}

   void open(UIElement *parent);
   void close();
   void setMaximized(bool set);
   UIElement *getDlgElement();

};