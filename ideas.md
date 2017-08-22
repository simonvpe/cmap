# Simplified and clarified API #
```c++
// MIT License

// Copyright (c) 2017 Simon Pettersson

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <utility>
#include <stdexcept>

namespace cmap {

namespace _model {
  using std::pair;

  constexpr auto make_terminal(std::pair<auto,auto> entry) {
    const auto [key, value] = entry;
    return [key,value](auto _key) {
      const auto result = (_key == key);
      return std::pair(result, value);
    };
  };

  constexpr auto make_branch(auto left, auto right) {
    return [left,right](auto key) {
        if(const auto [success, value] = left(key); success) {
          return pair(true, value);
        }
        if(const auto [success, value] = right(key); success) {
          return pair(true, value);
        }
        const auto dummy_value = left(key).second;
        return pair(false, dummy_value);
    };
  }

  constexpr auto merge(auto node) {
    return node;
  }

  constexpr auto merge(auto left, auto ... rest) {
    return make_branch(left, merge(rest...));
  }

}

// API Functions follow

constexpr auto make_map(std::pair<auto,auto> left_node, std::pair<auto,auto>...rest) {
  return _model::merge(
    _model::make_terminal(left_node), 
    _model::make_terminal(rest)...
  );
}

constexpr auto join(auto left_map, auto right_map) {
  return _model::merge(
    left_map, 
    right_map
  );
}

constexpr auto lookup(auto tree, auto key) {
  const auto [success, value] = tree(key);
  return success ? value : throw std::out_of_range("No such key");
}

auto foo() {
  constexpr auto a = std::pair<int,int>{12,42};
  constexpr auto b = std::pair<int,int>{13,43};
  constexpr auto map1 = make_map(a);
  constexpr auto map2 = make_map(b);
  constexpr auto map3 = join(map1, map2);
  constexpr auto val = lookup(map3, 12);
  return val;
}

} // namespace cmap
```
# Binary search enabled cmap #
```c++
#include <utility>
#include <stdexcept>

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

int main() {
  constexpr index<int,int> i0;
  constexpr auto i1 = i0.insert(5,12).insert(6,13).insert(2,17);
  volatile auto key = 5;
  const auto val = i1.find(key, binary_search<int>);
  return val;
}
```
