#pragma once

#include <unordered_map>
#include <vector>
#include <memory>

#include "Component.h"
#include "StringTable.h"
#include "Vector.h"

struct SkeletalFrame
{
   Float2 offset, rotationPoint;
   float angle, z;
   bool flipX, flipY;

   const long frame;

   SkeletalFrame(long frame):frame(frame), angle(0.0f), z(0.0f), flipX(false), flipY(false){}
};

struct SkeletalNode
{
   Entity * entity;
   Float2 pos;

   SkeletalNode(){}
   SkeletalNode(Float2 pos):pos(pos){}
};

struct SkeletalLimbComponent : public Component
{
   SkeletalLimbComponent():startFrameSet(false), skeletalParent(nullptr), z(0.0f){}
   std::unordered_map<InternString, SkeletalNode> nodes;
   float z;
   Entity *skeletalParent;

   //node this entity is attached to parent on
   InternString attachedNode;

   std::shared_ptr<SkeletalFrame> startFrame;
   bool startFrameSet;

   //helper functions
   const Float2 &getNodePosition(InternString name) const;

   const SkeletalNode &getNode_const(InternString name) const;
   SkeletalNode &getNode(InternString name);
   SkeletalNode &addNode(InternString name, const Float2 &position);
   void attachToNode(InternString nodeName, Entity *e);

};



struct SkeletalFrameObject
{
   std::string fullName;
   std::vector<InternString> name;
   std::vector<SkeletalFrame> frames;

   SkeletalFrameObject(std::string fullName, std::vector<InternString> name):
      fullName(std::move(fullName)), name(std::move(name)){}
};

class SkeletalAnimation
{
   std::vector<SkeletalFrameObject> m_frameObjs;

   bool m_loop;
   long m_frameCount;

public:
   SkeletalAnimation():m_frameCount(1){}

   void setLooping(bool loop){m_loop = loop;}
   void updateEntity(float timeElapsed, Entity *entity);
   SkeletalFrame &addFrame(std::string partName, long frame);

};