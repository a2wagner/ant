#pragma once

#include <vector>
#include <algorithm>

namespace ant {
namespace std_ext {

inline void insertRange(std::vector<unsigned>& v, unsigned start, unsigned stop) {
  int length = stop-start+1;
  if(length<1)
    return;
  std::vector<unsigned> v_(static_cast<size_t>(length));
  std::iota(v_.begin(), v_.end(), start);
  v.insert(v.end(), v_.cbegin(), v_.cend());
}

template<typename T>
inline bool contains(const std::vector<T> v, const T& val) {
  return std::find(v.cbegin(), v.cend(), val) != v.cend();
}

}} // namespace ant::std_ext