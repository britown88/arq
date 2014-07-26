#include "CoreComponents.h"
#include "Skeletal.h"

#include "GLFW\glfw3.h"

void TextureComponent::defaultBlend()
{
   blendS = GL_ONE;
   blendD = GL_ONE_MINUS_SRC_ALPHA;
}

void registerCoreComponents()
{
   static bool initialized = false;

   if(!initialized)
   {
      registerComponent<PositionComponent>();
      registerComponent<MeshComponent>();
      registerComponent<GraphicalBoundsComponent>();
      registerComponent<RotationComponent>();
      registerComponent<TextureComponent>();
      registerComponent<LayerComponent>();

      registerComponent<SkeletalLimbComponent>();

      initialized = true;
   }
   
}