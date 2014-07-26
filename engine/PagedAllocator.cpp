#include "PagedAllocator.h"
#include <chrono>
#include <thread>

bool SpinLock::tryLock()
{
   return _InterlockedCompareExchange(&locked, 1, 0) == 0;
}

void SpinLock::lock()
{
   int spins = 0;
   for(;;)
   {
      if(tryLock())
         return;

      if(spins > 25 && spins < 10000)
      {
         std::this_thread::yield();
      }
      else if(spins > 10000)
      {
         std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }

      ++spins;
   }
}

void SpinLock::unlock()
{
   locked = 0;
}