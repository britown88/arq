#include "engine\Entity.h"
#include "engine\IndexList.h"
#include "engine\VertexList.h"
#include "engine\Entity.h"
#include "engine\CoreComponents.h"
#include "engine\GameHelpers.h"
#include "engine\IOCContainer.h"
#include "engine\StringTable.h"
#include "engine\Application.h"
#include "engine\ComponentHelpers.h"
#include "engine\Skeletal.h"

Entity *buildBasicBodyPart(Entity *e, char *texture, Float2 size, Float2 center)
{
   auto vbo = GameHelpers::standardRectangleVBO();
   auto ibo = GameHelpers::standardRectangleIBO();
   auto st = IOC.resolve<StringTable>();

   e->add(MeshComponent(ibo, vbo, Colorf(1, 1, 1)));
   e->add(TextureComponent(st->get(texture)));
   e->add(GraphicalBoundsComponent(size, center));
   e->add(PositionComponent(Float2()));
   e->add(SkeletalLimbComponent());

   e->setNew();

   return e;
}

Entity *buildCharacter(EntitySystem &system)
{   
   auto &app = *IOC.resolve<Application>();
   auto st = IOC.resolve<StringTable>();

   //beautiful numbers
   float pixelMult = 6.0f;
   float torsoSize = 9.0f * pixelMult, headSize = 11.0f * pixelMult, rArmSize = 6.0f * pixelMult, 
         lArmSize =  6.0f * pixelMult, rLegSize = 6.0f * pixelMult,  lLegSize = 6.0f * pixelMult;
   Float2 headConn(4.5f * pixelMult, 3.0f * pixelMult);Float2 headCenter(5.5f * pixelMult, 11.0f * pixelMult);
   Float2 rArmConn(0.0f * pixelMult, 4.0f * pixelMult);Float2 rArmCenter(4.0f * pixelMult, 2.0f * pixelMult);
   Float2 lArmConn(9.0f * pixelMult, 4.0f * pixelMult);Float2 lArmCenter(2.0f * pixelMult, 2.0f * pixelMult);
   Float2 rLegConn(2.0f * pixelMult, 8.0f * pixelMult);Float2 rLegCenter(3.0f * pixelMult, 3.0f * pixelMult);
   Float2 lLegConn(7.0f * pixelMult, 8.0f * pixelMult);Float2 lLegCenter(3.0f * pixelMult, 3.0f * pixelMult);

   float charSize = 25.0f;
   Float2 charCenter = Float2(charSize * 0.5f, charSize * 0.5f);

   auto sTorso = st->get("torso");
   auto sHead = st->get("head");
   auto sLeftArm = st->get("leftArm");
   auto sRightArm = st->get("rightArm");
   auto sLeftLeg = st->get("leftLeg");
   auto sRightLeg = st->get("rightLeg");

   auto vbo = GameHelpers::standardRectangleVBO();
   auto ibo = GameHelpers::standardRectangleIBO();

   auto e = system.createEntity();

   e->add(PositionComponent(Float2()));
   e->add(GraphicalBoundsComponent(Float2(charSize, charSize), charCenter));
   //e->add(MeshComponent(ibo, vbo, Colorf(1, 1, 1, 0)));
   e->add(RotationComponent(0.0f, Float2()));
   //e->add(TextureComponent(st->get("assets/img/00.png")));
   e->add(SkeletalLimbComponent());

   e->setNew();

   Entity *torso = buildBasicBodyPart(system.createEntity(), 
                     "assets/img/body/torso/front/00.png", 
                     Float2(torsoSize, torsoSize), 
                     Float2(torsoSize*0.5f, torsoSize*0.5f));

   //torso->remove<MeshComponent>();
   //torso->setChanged();

   SkeletalLimbComponent torsoSkel;
   torsoSkel.addNode(sHead, headConn);
   torsoSkel.addNode(sRightArm, rArmConn);
   torsoSkel.addNode(sLeftArm, lArmConn);
   torsoSkel.addNode(sRightLeg, rLegConn);
   torsoSkel.addNode(sLeftLeg, lLegConn);
   torso->add(torsoSkel);

   if(auto skel = e->lock<SkeletalLimbComponent>())
   {
      skel->addNode(sTorso, Float2(charCenter.x, charCenter.y + 2.5f));
      skel->attachToNode(sTorso, torso);
   }

   auto head = buildBasicBodyPart(system.createEntity(), "assets/img/body/head/front/00.png", Float2(headSize, headSize), headCenter);
   auto rArm = buildBasicBodyPart(system.createEntity(), "assets/img/body/arm.png", Float2(rArmSize, rArmSize), rArmCenter);
   auto lArm = buildBasicBodyPart(system.createEntity(), "assets/img/body/arm.png", Float2(lArmSize, lArmSize), lArmCenter);
   auto rLeg = buildBasicBodyPart(system.createEntity(), "assets/img/body/leg.png", Float2(rLegSize, rLegSize), rLegCenter);
   auto lLeg = buildBasicBodyPart(system.createEntity(), "assets/img/body/leg.png", Float2(lLegSize, lLegSize), lLegCenter);

   if(auto tc = lArm->lock<TextureComponent>())
      tc->flipX = true;

   if(auto tc = lLeg->lock<TextureComponent>())
      tc->flipX = true;

   if(auto sc = head->lock<SkeletalLimbComponent>())sc->z = 1.0f;
   if(auto sc = rArm->lock<SkeletalLimbComponent>())sc->z = -2.0f;
   if(auto sc = lArm->lock<SkeletalLimbComponent>())sc->z = -2.0f;
   if(auto sc = rLeg->lock<SkeletalLimbComponent>())sc->z = -1.0f;
   if(auto sc = lLeg->lock<SkeletalLimbComponent>())sc->z = -1.0f;


   if(auto tSkel = torso->lock<SkeletalLimbComponent>())
   {
      tSkel->attachToNode(sHead, head);
      tSkel->attachToNode(sRightArm, rArm);
      tSkel->attachToNode(sLeftArm, lArm);
      tSkel->attachToNode(sRightLeg, rLeg);
      tSkel->attachToNode(sLeftLeg, lLeg);
   }

   

   return e;
}

