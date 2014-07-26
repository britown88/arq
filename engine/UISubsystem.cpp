#include "UISubsystem.h"

class BasicSubsystem : public SubsystemElement
{
public:
   BasicSubsystem(std::unique_ptr<UIElement> inner)
      :SubsystemElement(std::move(inner)){}
};

std::unique_ptr<SubsystemElement> buildBasicSubsystem(std::unique_ptr<UIElement> inner)
{
   return std::unique_ptr<SubsystemElement>(new BasicSubsystem(std::move(inner)));
}