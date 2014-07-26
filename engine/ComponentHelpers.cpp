#include "ComponentHelpers.h"

#include "CoreComponents.h"
#include "Skeletal.h"

Entity *ComponentHelpers::getBaseParent(Entity *e)
{
   if(auto skel = e->get<SkeletalLimbComponent>())
   {
      if(skel->skeletalParent)
         return getBaseParent(skel->skeletalParent);
   }

   return e;
}

float ComponentHelpers::getSkeletalZ(Entity *e)
{
   if(auto skel = e->get<SkeletalLimbComponent>())
   {
      if(skel->skeletalParent)
         return skel->z + getSkeletalZ(skel->skeletalParent);

      return skel->z;
   }

   return 0.0f;
}

float ComponentHelpers::getBottomY(Entity *e)
{
   float bottomY = 0;
   auto pos = e->get<PositionComponent>();
   auto gbc = e->get<GraphicalBoundsComponent>();

   if(pos)
      bottomY += pos->pos.y;
   if(gbc)
   {
      bottomY += gbc->size.y;
      bottomY -= gbc->center.y;
   }

   return bottomY;

}
int ComponentHelpers::getLayer(Entity *e)
{
   if(auto l = e->get<LayerComponent>())
      return l->layer;

   return 0;
}

void ComponentHelpers::buildEntityTextureTransform(Entity *e, Matrix &texMatrix)
{
   MatrixTransforms::identity(texMatrix);

   if(auto tc = e->get<TextureComponent>())
	{
		if(tc->flipX)
		{
			texMatrix[12] = 1.0f;
			texMatrix[0] = -1.0f;				
		}				
			
		if(tc->flipY)
		{
			texMatrix[13] = 1.0f;
			texMatrix[5] = -1.0f;
		}
			
      if(auto size = tc->size)
		{
			float sX = 1.0f, sY = 1.0f;
			float xSize = 0.0f;
			float ySize = 0.0f;
					
         if(auto gbc = e->get<GraphicalBoundsComponent>())
			{
				sX = gbc->size.x;
				sY = gbc->size.y;
			}
					
			xSize = sX / size->x;
			ySize = sY / size->y;

			texMatrix[0] *= xSize;
			texMatrix[1] *= xSize;
				
			texMatrix[4] *= ySize;
			texMatrix[5] *= ySize;
      }
	}
}

Rectf ComponentHelpers::getEntityRect(Entity *e)
{
   auto pc = e->get<PositionComponent>();
   auto gbc = e->get<GraphicalBoundsComponent>();

   Float2 pos;
   Float2 size(1, 1);

   if(pc)
   {
      pos.x = pc->pos.x;
      pos.y = pc->pos.y;
   }

   if(gbc)
   {
      size.x = gbc->size.x;
      size.y = gbc->size.y;

      pos.x -= gbc->center.x;
	   pos.y -= gbc->center.y;
   }

   return Rectf(pos.x, pos.y, pos.x + size.x, pos.y + size.y);
}
