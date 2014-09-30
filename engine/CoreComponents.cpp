#include "CoreComponents.h"

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
      registerComponent<CollisionBoxComponent>();
      registerComponent<MeshComponent>();
      registerComponent<GraphicalBoundsComponent>();
      registerComponent<SpriteComponent>();
      registerComponent<CenterComponent>();
      registerComponent<RotationComponent>();
      registerComponent<TextureComponent>();
      registerComponent<LayerComponent>();

      initialized = true;
   }
   
}