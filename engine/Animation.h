#pragma once

#include <memory>
#include <vector>
#include "boost\optional.hpp"

template <typename T>
class IAnimation
{
 public:
   virtual ~IAnimation() {}
   virtual T get(float timeOffset)=0;
   virtual boost::optional<float> animTime()=0; //returns boost::none for infinite loops
};

template <typename T>
std::unique_ptr<IAnimation<T> > vectorToAnimation(std::vector<T> vec, float timePerFrame)
{
   class Anim : public IAnimation<T>
   {
      std::vector<T> m_vec;
      float m_timePerFrame;
   public:
      Anim(std::vector<T> vec, float timePerFrame):m_vec(std::move(vec)), m_timePerFrame(timePerFrame){}

      T get(float timeOffset)
      {
         int index = timeOffset / m_timePerFrame;
         if(index < m_vec.size())
            return m_vec[index];

         return m_vec.back();
      }

      boost::optional<float> animTime() 
      {
         return m_timePerFrame * m_vec.size();
      }
   };

   return std::unique_ptr<IAnimation<T> >(new Anim(std::move(vec), timePerFrame));
}

template <typename T>
std::unique_ptr<IAnimation<T> > loopAnimation(std::unique_ptr<IAnimation<T> > anim)
{
   class LoopedAnim : public IAnimation<T>
   {
      std::unique_ptr<IAnimation<T> > m_anim;
   public:
      LoopedAnim(std::unique_ptr<IAnimation<T> > anim):m_anim(std::move(anim)){}
      T get(float t) {return m_anim->get(fmodf(t, *m_anim->animTime()));}
      boost::optional<float> animTime() {return boost::none;}
   };

   return std::unique_ptr<IAnimation<T> >(new LoopedAnim(std::move(anim))); 
}