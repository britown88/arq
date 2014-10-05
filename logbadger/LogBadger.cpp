#include "LogBadger.h"

#include "engine\CoreUI.h"
#include "engine\IOCContainer.h"
#include "engine\Logs.h"

#include <vector>

static const int LineCount = 32;

class LogBadger : public Tool
{

   std::vector<CoreUI::TextElement*> m_lines;

   LogManager &lm;
public:
   LogBadger():lm(*IOC.resolve<LogManager>()){}
   ~LogBadger(){}

   std::string getTitle(){return "LogBadger";}
   Float2 getStartingPosition(){return Float2(1000.0f, 50.0f);}
   Float2 getStartingSize(){return Float2(400.0f, 800.0f);}

   void onOpen()
   {
      auto height = 20;

      auto font = std::make_shared<TextFont>("assets/fonts/retganon.ttf", 12.0f);

      for(int i = 0; i < LineCount; ++i)
      {
         auto elem = getDlgElement()->intern(CoreUI::buildTextElement("", font, Float2(0, height * (i+1))));
         m_lines.push_back(elem);
         getDlgElement()->pushListChild(elem);
      }

      getDlgElement()->arrange();

      Logs::d("LogBadger") << "Welcome to LogBadger!";
   }

   void onStep()
   {
      auto logs = lm.getLogs();
      int logSize = logs.size();

      int line = std::min(LineCount - 1, logSize - 1);
      for(auto iter = logs.rbegin(); iter != logs.rend() && line >= 0; ++iter)
      {
         switch(iter->level)
         {
         case LogLevel::Debug:
            m_lines[line]->setColor(Colorf(0, 1, 0));
            break;
         case LogLevel::Info:
            m_lines[line]->setColor(Colorf(1, 1, 1));
            break;
         case LogLevel::Warning:
            m_lines[line]->setColor(Colorf(1, 1, 0));
            break;
         case LogLevel::Error:
            m_lines[line]->setColor(Colorf(1, 0, 0));
            break;
         };

         m_lines[line]->setText(iter->msg);

         --line;
      }

   }

   void onClose()
   {
   }
};

std::unique_ptr<Tool> createLogBadger()
{
   return std::unique_ptr<Tool>(new LogBadger());
}