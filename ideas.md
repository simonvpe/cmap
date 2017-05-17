#Binary search enabled cmap#
```c++
#include <utility>
#include <stdexcept>

template<typename T>
constexpr auto binary_search(const T* first, const T* last, T key) {
  const auto size = last - first;
  auto left  = 0;
  auto right = size - 1;
  while(left <= right) {
    const auto m       = (left + right) / 2;
    const auto current = first[m];
    left               = (current < key) ? m + 1 : left;
    right              = (current > key) ? m - 1 : right;
    if(current == key) return m;
  }
  return ptrdiff_t{-1};
}

template<typename TKey, typename TValue, int TSize = 0>
struct index {
  using pair_t = std::pair<TKey,TValue>;

  TKey   keys[TSize];
  TValue values[TSize];

  constexpr index() 
  : keys{}
  , values{}
  {}

  constexpr index(index<TKey,TValue,TSize-1> prev, TKey key, TValue value)
  : keys{}
  , values{}
  {
    // This code copies the values from the previous (smaller) index and
    // at the same time inserts the key/value combination at the corret
    // spot
    auto i = 0;
    while(i < (TSize - 1) && prev.keys[i] < key) {
      keys[i]   = prev.keys[i];
      values[i] = prev.values[i];
      ++i;
    }
    keys[i]   = key;
    values[i] = value;
    while(i < (TSize - 1)) {
      keys[i+1]   = prev.keys[i];
      values[i+1] = prev.values[i];
      ++i;
    }
  }

  constexpr auto insert(TKey key, TValue value) const
  {
    using  next_t = index<TKey,TValue,TSize+1>;
    return next_t(*this, key, value);
  }

  constexpr auto find(TKey key) const {
    const auto first = keys;
    const auto last  = first + TSize;
    const auto idx   = binary_search(first, last, key);
    return idx >= 0 ? values[idx] : throw std::out_of_range("Key not found!");
  }
};

int main() {
  constexpr index<int,int> i0;
  constexpr auto i1 = i0.insert(5,12).insert(6,13).insert(2,17);
  constexpr auto key = 5;
  constexpr auto val = i1.find(9);
  return val;
}
```
