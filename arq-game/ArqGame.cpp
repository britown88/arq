#include "ArqGame.h"
#include "engine\Tool.h"
#include "engine\IndexList.h"
#include "engine\VertexList.h"
#include "engine\Entity.h"

#include "engine\CoreComponents.h"
#include "engine\GameHelpers.h"
#include "engine\IOCContainer.h"
#include "engine\StringTable.h"
#include "engine\Application.h"
#include "engine\CoreUI.h"
#include "engine\ComponentHelpers.h"
#include "engine\MatrixManager.h"
#include "engine\DataManager.h"
#include "engine\SpriteManager.h"
#include "engine\Logs.h"

#include "ArqComponents.h"
#include "PhysicsManager.h"
#include "InputManager.h"
#include "CharacterManager.h"
#include "RenderManager.h"
#include "PartitionManager.h"

#include "GridEditor.h"
#include "GridManager.h"

#include <unordered_map>

class ArqGame : public Tool
{
   EntitySystem m_system;

   std::vector<Entity *> m_entities;
   Application &app;

   CoreUI::WorldUIElement *m_worldElement;

   std::unordered_map<std::string, CharacterComponent> m_characterTemplates;
   

   void buildBlock(Recti cellRange)
   {
      auto vbo = GameHelpers::standardRectangleVBO();
      auto ibo = GameHelpers::standardRectangleIBO();

      auto st = IOC.resolve<StringTable>();

      auto e = m_system.createEntity();
      e->add(PositionComponent());
      e->add(GraphicalBoundsComponent());
      e->add(CollisionBoxComponent());
      e->add(MeshComponent(ibo, vbo, Colorf(1, 1, 1)));
      e->add(TextureComponent(st->get("assets/img/block.png")));

      e->add(GridComponent(cellRange));

      e->setNew();

   }

   void buildTestEntities()
   {
      auto vbo = GameHelpers::standardRectangleVBO();
      auto ibo = GameHelpers::standardRectangleIBO();

      auto st = IOC.resolve<StringTable>();

      for(int i = 0; i <= 100; ++i) buildBlock(Recti(i, 21, i+1, 22));//floor
      for(int i = 0; i <= 20; ++i) buildBlock(Recti(0, i, 1, i+1));//left wall
      for(int i = 0; i <= 20; ++i) buildBlock(Recti(100, i, 101, i+1));//right wall

      for(int i = 0; i <= 5; ++i) buildBlock(Recti(10+i, 11, 11+i, 12));
      for(int i = 0; i <= 5; ++i) buildBlock(Recti(7+i, 15, 8+i, 16));
      for(int i = 0; i <= 5; ++i) buildBlock(Recti(15+i, 18, 16+i, 19));
      

      for(int i = 0; i < 3; ++i)
      {
         float size = 128;
         auto e = m_system.createEntity();
         e->add(PositionComponent(Float2(100 * (i+1), 0)));
         e->add(GraphicalBoundsComponent(Float2(size, size)));
         //e->add(CollisionBoxComponent(0, 0, size, size));
         e->add(CollisionBoxComponent(44, 64, 40, 60));
         e->add(MeshComponent(ibo, vbo, Colorf(1, 1, 1)));
         e->add(RotationComponent(0.0f, Float2(size/2.0f, size/2.0f)));
         e->add(TextureComponent());

         e->add(VelocityComponent(Float2(50.0f, 50.0f)));

         auto iter = m_characterTemplates.find("sirlayersworth");
         if(iter != m_characterTemplates.end())
         {
            CharacterComponent cc = iter->second;
            cc.playerNumber = i;

            e->add(SpriteComponent(cc.idleSprite));
            e->add(cc);
         }

         //e->lock<VelocityComponent>()->velocity.x = 10.0f;

         e->setNew();
         m_entities.push_back(e);
         
      }

      
   }
   void buildBackground()
   {
      //bg
      //auto bg = m_system.createEntity();
      //bg->add(PositionComponent(Float2()));
      //bg->add(GraphicalBoundsComponent(Float2(2000, 2000)));
      //bg->add(MeshComponent(ibo, vbo, Colorf(1, 1, 1)));
      //bg->add(LayerComponent(-1));
      //TextureComponent bgTex(st->get("assets/img/sand.png"));
      //bgTex.repeatType = RepeatType::Repeat;
      //bgTex.size = Float2(450, 450);
      //bg->add(bgTex);
      //bg->setNew();
   }

   void initManagers()
   {
      m_system.addManager(buildRenderManager());
      m_system.addManager(buildGraphicalPartitionManager());
      m_system.addManager(buildCollisionPartitionManager());
      m_system.addManager(buildMatrixManager());
      m_system.addManager(buildPhysicsManager());
      m_system.addManager(buildInputManager(getDlgElement()));
      m_system.addManager(buildCharacterManager());
      m_system.addManager(buildGridManager());
   }

   void initDataManager()
   {
      auto dm = IOC.resolve<DataManager>();
      dm->registerDataTypeLoader("sprite", [&](Property &p)
      {
         if(auto name = p["name"].get<std::string>())
         if(auto anim = buildSpriteAnimation(p))
         {
            IOC.resolve<SpriteManager>()->addSprite(*name, std::move(anim));            
         }

      });

      dm->registerDataTypeLoader("character", [&](Property&p)
      {
         if(auto name = p["name"].get<std::string>())
         {
            auto st = IOC.resolve<StringTable>();
            CharacterComponent cc;
            if(auto str = p["runsprite"].get<std::string>()) cc.runSprite = st->get(*str);
            if(auto str = p["idlesprite"].get<std::string>()) cc.idleSprite = st->get(*str);
            if(auto str = p["jumpupsprite"].get<std::string>()) cc.jumpUpSprite = st->get(*str);
            if(auto str = p["jumpdownsprite"].get<std::string>()) cc.jumpDownSprite = st->get(*str);

            m_characterTemplates.insert(std::make_pair(*name, cc));
         }
      });

      dm->loadData("assets/data");
   }

public:
   ArqGame():app(*IOC.resolve<Application>()){}

   std::string getTitle(){return "WHY IS IT TWICE!";}
   Float2 getStartingPosition(){return Float2();}
   Float2 getStartingSize(){return Float2(800.0f, 600.0f);}
   
   void onOpen()
   {
      initManagers();   
      initDataManager();

      m_system.getManager<GridManager>()->createGrid(Float2(), Int2(200, 100), Float2(32.0f, 32.0f));

      buildTestEntities();

      m_worldElement = getDlgElement()->intern<CoreUI::WorldUIElement>(CoreUI::buildWorldUIElement(Rectf(), m_system.getManager<RenderManager>()));
      m_worldElement->cameraBounds() = Rectf(0, 0, 1920, 1080);
      m_worldElement->anchorToParent();

      getDlgElement()->intern(createGridEditor(m_worldElement, &m_system));


      getDlgElement()->pushListChild(m_worldElement);      
      
   }

   void onStep()
   {

      m_system.getManager<CharacterManager>()->update();
      m_system.getManager<PhysicsManager>()->update();
      
   }

   void onClose()
   {
   }
};

std::unique_ptr<Tool> createArqGame()
{
   return std::unique_ptr<Tool>(new ArqGame());
}