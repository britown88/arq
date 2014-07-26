#pragma once

#include <algorithm>

//[11:16:13 PM] Sean Hunter: 
template <typename T>
T seanSort(T cont)
{
  std::sort(begin(cont), end(cont));
  return cont;
}

template <typename T, typename Fn>
T seanSort(T cont, Fn&& fn)
{
  std::sort(begin(cont), end(cont), std::forward<Fn>(fn));
  return cont;
}