#include <nonius/nonius.h++>

#include <utility>
#include <stdexcept>
#include <map>

#include "pcg.hpp"

namespace cmap {
  
template<typename T>
constexpr inline auto binary_search = [](const T* first, const T* last, T key) {
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
};

template<typename T>
constexpr inline auto linear_search = [](const T* first, const T* last, T key) {
  const auto size = last - first;
  for(auto i = 0 ; i < size ; ++i) {
    if(first[i] == key) return ptrdiff_t{i};
  }
  return ptrdiff_t{-1};
};

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
  
  template<typename TSearch>
  constexpr auto find(TKey key, TSearch&& search) const {
    const auto first = keys;
    const auto last  = first + TSize;
    const auto idx   = search(first, last, key);
    return idx >= 0 ? values[idx] : throw std::out_of_range("Key not found!");
  }
};

template<typename TKey, typename TValue, int TSize>
constexpr auto operator<<(index<TKey,TValue,TSize> left, std::pair<TKey,TValue> p) {
  const auto [key, value] = p;
  return left.insert(key, value);
}
  
constexpr auto map(auto key, auto value) {
  return std::pair(key, value);
}

template<typename TKey, typename TValue>
constexpr auto make_map(std::pair<TKey,TValue> first) {
  return index<TKey,TValue>() << first;
}

template<typename TKey, typename TValue, typename...Ts>
constexpr auto make_map(std::pair<TKey,TValue> first, Ts...rest) {
  return index<TKey,TValue>() << first << (... << rest);
}
  
} // namespace cmap

template<std::size_t TSize>
constexpr auto random() {
  PCG pcg;
  pcg(); // initialize
  std::array<int, TSize> numbers{};
  for(std::size_t i = 0 ; i < TSize ; ++i) {
    numbers[i] = pcg();
  }
  return numbers;
}

template<auto TSize>
constexpr auto reverse(const std::array<int,TSize> in) {
  std::array<int,TSize> out;
  for(auto i = 0 ; i < in.size() ; ++i) {
    out[i] = in[in.size() - 1 - i];
  }
  return out;
}

template<std::size_t TSize>
auto make_map(std::array<int, TSize> keys, std::array<int, TSize> values) {
  std::map<int,int> map;
  for(std::size_t i = 0 ; i < TSize ; ++i) {
    map[keys[i]] = values[i];
  }
  return map;
}

template<std::size_t TSize, std::size_t TDepth = 0>
constexpr auto generate_random_lookup(const std::array<int,TSize>& keys, const std::array<int,TSize>& values) {
  if constexpr ((TSize - 1) == TDepth) {
    cmap::index<int,int> root;
    return cmap::make_map(cmap::map(keys[TDepth], values[TDepth]));
  }
  else {
    return generate_random_lookup<TSize,TDepth + 1>(keys, values) << std::pair(keys[TDepth], values[TDepth]);
  }
}

template<auto TSize> constexpr auto keys          = random<TSize>();
template<auto TSize> constexpr auto values        = random<TSize>();
template<auto TSize> constexpr auto lookup        = generate_random_lookup(keys<TSize>,values<TSize>);
template<auto TSize> const auto map               = make_map(keys<TSize>, values<TSize>);
template<auto TSize> constexpr auto reversed_keys = reverse(keys<TSize>);

NONIUS_BENCHMARK("[random access 100 (binary_search, constexpr)]", []{
    for(const auto key : reversed_keys)) {
      volatile auto value = lookup<100>.find(key, cmap::binary_search<int>);
      value;
    }
})

NONIUS_BENCHMARK("[random access 100 (binary_search, volatile)]", []{
    for(const auto key : reversed_keys) {
      volatile auto k = key;
      volatile auto value = lookup<100>.find(k, cmap::binary_search<int>);
      value;
    }
})

NONIUS_BENCHMARK("[random access 100 (linear_search, constexpr)]", []{
    for(const auto key : reversed_keys) {
      volatile auto value = lookup<100>.find(key, cmap::linear_search<int>);
      value;
    }
})

NONIUS_BENCHMARK("[random access 100 (linear_search, volatile)]", []{
    for(const auto key : reversed_keys) {
      volatile auto k = key;
      volatile auto value = lookup<100>.find(k, cmap::linear_search<int>);
      value;
    }
})

NONIUS_BENCHMARK("[random access 100 (std::map)]", []{
    for(const auto key : reversed_keys) {
      volatile auto value = map<100>.at(key);
      value;
    }
})
