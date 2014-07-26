#include "ArqApp.h"
#include "engine\IOCContainer.h"

int main(void)
{
   IOC.add<Application>(buildArqApp());
   auto app = IOC.resolve<Application>();

   app->start();

   return 0;
}