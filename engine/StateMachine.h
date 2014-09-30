#pragma once

#include <memory>

template <typename State>
class StateMachine
{
   std::vector<std::unique_ptr<State> > stateStack;
   std::unique_ptr<State> queued;
   typedef void (StateMachine::*bool_type)() const;
   void this_type_does_not_support_comparisons() const {}
   void pushQueue()
   {
      if (queued)
      {
         stateStack.clear();
         stateStack.push_back(std::move(queued));
      }
   }
public:
   void push(State* state)
   {
      push(std::unique_ptr<State>(state));
   }
   void push(std::unique_ptr<State> state)
   {
      if(!stateStack.empty()) 
         stateStack.back()->onLeave();

      pushQueue();
      stateStack.push_back(std::move(state));
      stateStack.back()->onEnter();
   }
   void pop()
   {
      if (queued)
      {
         queued.reset();
         return;
      }
      if (!stateStack.empty()) 
      {
         stateStack.back()->onLeave();
         stateStack.pop_back();
      }

      if (!stateStack.empty()) 
      {
         stateStack.back()->onEnter();
      }
   }
   void set(State* state)
   {
      set(std::unique_ptr<State>(state));
   }
   void set(std::unique_ptr<State> state)
   {
      clear();
      push(std::move(state));
   }
   void queue(State* state)
   {
      queue(std::unique_ptr<State>(state));
   }
   void queue(std::unique_ptr<State> state)
   {
      pushQueue();
      queued = std::move(state);
   }

   void clear()
   {
      while (!stateStack.empty()) 
      {
         stateStack.back()->onLeave();
         stateStack.pop_back();
      }
         

      queued.reset();

   }
   State* get()
   {
      pushQueue();
      if (!empty()) return stateStack.back().get();
      return nullptr;
   }
   State* operator->()
   {
      return get();
   }
   bool empty() const
   {
      return stateStack.empty() && !queued;
   }
   operator bool_type() const {
      return !empty() ? 
         &StateMachine::this_type_does_not_support_comparisons : 0;
   }
   
};
