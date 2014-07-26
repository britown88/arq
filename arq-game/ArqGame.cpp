#include "ArqGame.h"
#include "engine\Tool.h"
#include "engine\IndexList.h"
#include "engine\VertexList.h"
#include "engine\Entity.h"
#include "engine\RenderManager.h"
#include "engine\PartitionManager.h"
#include "engine\CoreComponents.h"
#include "engine\GameHelpers.h"
#include "engine\IOCContainer.h"
#include "engine\StringTable.h"
#include "engine\Application.h"
#include "engine\CoreUI.h"
#include "engine\ComponentHelpers.h"
#include "engine\MatrixManager.h"
#include "engine\Skeletal.h"

Entity *buildCharacter(EntitySystem &system);

class ArqGame : public Tool
{
   std::shared_ptr<IBO> ibo;
   std::shared_ptr<VBO> vbo;

   EntitySystem m_system;
   RenderManager *m_renderManager;
   PartitionManager *m_partitionManager;
   MatrixManager *m_matrixManager;
   std::vector<Entity *> m_entities;
   Application &app;

   const int m_renderPartIndex;
   CoreUI::WorldUIElement *m_worldElement;

   void buildTestEntities()
   {
      vbo = GameHelpers::standardRectangleVBO();
      ibo = GameHelpers::standardRectangleIBO();

      auto st = IOC.resolve<StringTable>();

      for(int i = 0; i < 100; ++i)
      {
         float size = 50;
         auto e = m_system.createEntity();
         e->add(PositionComponent(Float2(app.rand(0, 1500), app.rand(0, 1000))));
         e->add(GraphicalBoundsComponent(Float2(size, size)));
         e->add(MeshComponent(ibo, vbo, Colorf(1, 1, 1)));
         e->add(RotationComponent(0.0f, Float2(size/2.0f, size/2.0f)));
         e->add(TextureComponent(st->get("assets/img/00.png")));
         e->add(SkeletalLimbComponent());

         e->setNew();
         m_entities.push_back(e);

         auto child = m_system.createEntity();
         child->add(PositionComponent(Float2()));

         child->add(GraphicalBoundsComponent(Float2(10, 10), Float2()));
         child->add(MeshComponent(ibo, vbo, Colorf(1, 1, 1, 1)));
         
         child->add(SkeletalLimbComponent());
         child->setNew();

         auto eSkel = e->lock<SkeletalLimbComponent>();
         auto cSkel = child->lock<SkeletalLimbComponent>();

         auto nodeName = st->get("test");
         eSkel->nodes.insert(std::make_pair(nodeName, SkeletalNode(Float2())));
         eSkel->nodes.at(nodeName).entity = child;

         cSkel->skeletalParent = e;
         cSkel->attachedNode = nodeName;
         cSkel->z = 1.0f;
         
      }
   }

public:
   ArqGame():app(*IOC.resolve<Application>()), m_renderPartIndex(getNewPartitionSelectionIndex()){}

   std::string getTitle(){return "Arq";}
   Float2 getStartingPosition(){return Float2();}
   Float2 getStartingSize(){return Float2(800.0f, 600.0f);}

   void onOpen()
   {
      vbo = GameHelpers::standardRectangleVBO();
      ibo = GameHelpers::standardRectangleIBO();

      auto st = IOC.resolve<StringTable>();


      m_renderManager = m_system.addManager(buildRenderManager(m_renderPartIndex));
      m_partitionManager = m_system.addManager(buildPartitionManager());
      m_matrixManager = m_system.addManager(buildMatrixManager());

      //bg
      auto bg = m_system.createEntity();
      bg->add(PositionComponent(Float2()));
      bg->add(GraphicalBoundsComponent(Float2(2000, 2000)));
      bg->add(MeshComponent(ibo, vbo, Colorf(1, 1, 1)));
      bg->add(LayerComponent(-1));
      TextureComponent bgTex(st->get("assets/img/sand.png"));
      bgTex.repeatType = RepeatType::Repeat;
      bgTex.size = Float2(450, 450);
      bg->add(bgTex);
      bg->setNew();

      auto e = buildCharacter(m_system);

      if(auto ePos = e->lock<PositionComponent>())
         ePos->pos = Float2(400, 400);
      
      //m_partitionManager->getEntities(Rectf(0,0,100,100), m_renderPartIndex);

      m_worldElement = getDlgElement()->intern<CoreUI::WorldUIElement>(CoreUI::buildWorldUIElement(Rectf(), m_renderManager));
      m_worldElement->cameraBounds() = Rectf(0, 0, 1280, 720);

      m_worldElement->anchorToParent();
      getDlgElement()->pushListChild(m_worldElement);
   }

   void onStep()
   {
      //for(auto e : m_entities)
      //{
      //   if(auto rot = e->lock<RotationComponent>())
      //   {
      //      rot->angle += 1.0f * app.dt();
      //   }
      //}

      m_partitionManager->getEntities(m_worldElement->cameraBounds(), m_renderPartIndex);
   }

   void onClose()
   {
   }
};

std::unique_ptr<Tool> createArqGame()
{
   return std::unique_ptr<Tool>(new ArqGame());
}