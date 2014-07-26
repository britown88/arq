#include "Skeletal.h"
#include "GnUtilities.h"
#include "CoreComponents.h"
#include <boost/algorithm/string.hpp>
#include "IOCContainer.h"

const Float2 &SkeletalLimbComponent::getNodePosition(InternString name) const
{
   auto node = nodes.find(name);
   if(node == nodes.end())
      throw ArqException("Invalid skeletal node name");

   return node->second.pos;
}

const SkeletalNode &SkeletalLimbComponent::getNode_const(InternString name) const
{
   auto node = nodes.find(name);
   if(node == nodes.end())
      throw ArqException("Invalid skeletal node name");

   return node->second;
}

SkeletalNode &SkeletalLimbComponent::getNode(InternString name)
{
   auto node = nodes.find(name);
   if(node == nodes.end())
      throw ArqException("Invalid skeletal node name");

   return node->second;
}

SkeletalNode &SkeletalLimbComponent::addNode(InternString name, const Float2 &position)
{
   nodes.insert(std::make_pair(name, SkeletalNode(position)));
   return nodes.at(name);
}

void SkeletalLimbComponent::attachToNode(InternString nodeName, Entity *e)
{
   if(auto skel = e->lock<SkeletalLimbComponent>())
   {
      getNode(nodeName).entity = e;
      skel->skeletalParent = parent;
      skel->attachedNode = nodeName;

   }
   else
      throw ArqException("Attempted to attach a skeleton-less entity to a skeletal node");
}


//skeletal animation
SkeletalFrame &SkeletalAnimation::addFrame(std::string partName, long frame)
{
   auto st = IOC.resolve<StringTable>();
   m_frameCount = std::max(m_frameCount, frame);

   SkeletalFrameObject *object = nullptr;
   for(auto &obj : m_frameObjs)
      if(obj.fullName == partName)
      {
         object = &obj;
         break;
      }

   if(!object)
   {
      std::vector<std::string> nameList;
      boost::split(nameList, partName, boost::is_any_of("/"));
      std::vector<InternString> internNameList;
      for(auto &name : nameList)
         internNameList.push_back(st->get(name.c_str()));

      m_frameObjs.push_back(SkeletalFrameObject(partName, std::move(internNameList)));
      object = &(m_frameObjs.back());
   }

   object->frames.push_back(SkeletalFrame(frame));

   return object->frames.back();

}

Entity *getLimbAt(Entity *e, SkeletalFrameObject &obj)
{
   auto snc = e->get<SkeletalLimbComponent>();
   if(!snc) return nullptr;

   auto partCount = obj.name.size();

   for(int i = 0; i < partCount; ++i)
   {
      auto node = snc->nodes.find(obj.name[i]);
      if(node != snc->nodes.end())//found node
      {
         auto child = node->second.entity;

         if(i == partCount - 1)
            return child;
         
         if(!child)
            return nullptr;

         snc = child->get<SkeletalLimbComponent>();
         if(!snc)
            return nullptr;
      }
      else
      {
         return nullptr;
      }
   }

   return nullptr;

}

void SkeletalAnimation::updateEntity(float timeElapsed, Entity *e)
{
   long frame = (long)(timeElapsed);

   if(frame > m_frameCount)
      if(m_loop)
         frame = frame % m_frameCount;
      else
         frame = m_frameCount;

   //for every frame object
   for(auto && obj : m_frameObjs)
   {
      //find the child entity
      auto child = getLimbAt(e, obj);

      if(!child)
         continue;

      //find the current frame
      SkeletalFrame *skelFrame = nullptr;
      int frameCount = obj.frames.size();
      for(int i = 0; i < frameCount - 1; ++i)
      {
         if(obj.frames[i].frame > frame || obj.frames[i].frame <= frame && obj.frames[i+1].frame > frame)
         {
            //frame found!
            skelFrame = &obj.frames[i];

            if(auto snc = child->lock<SkeletalLimbComponent>())
            {
               SkeletalFrame *frame1, *frame2;

               //if we're on the first frame
               if(skelFrame->frame > frame)
               {
                  if(!snc->startFrameSet)
                  {
                     //copy current data into entity's startFrame
                     snc->startFrame = std::make_shared<SkeletalFrame>(0);
                     snc->startFrame->z = snc->z;

                     if(auto pc = child->get<PositionComponent>())
                        snc->startFrame->offset = pc->pos;

                     if(auto rc = child->get<RotationComponent>())
                     {
                        snc->startFrame->rotationPoint = rc->point;
                        snc->startFrame->angle = rc->angle;
                     }

                     if(auto tc = child->get<TextureComponent>())
                     {
                        snc->startFrame->flipX = tc->flipX;
                        snc->startFrame->flipY = tc->flipY;
                     }

                     snc->startFrameSet = true;
                  }
            
                  frame2 = skelFrame;
                  frame1 = snc->startFrame.get();

               }
               else
               {
                  frame1 = skelFrame;
                  frame2 = &obj.frames[i+1];
                  snc->startFrameSet = false;
               }

               //interpolate into entity's currentFrame


            }
         }
      }

      
   }
}
