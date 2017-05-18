#include <nonius/nonius.h++>
#include <cmap.hpp>

#include <utility>
#include <stdexcept>

#include <cstdint>
#include <limits>


constexpr auto seed()
{
  std::uint64_t shifted = 0;

  for( const auto c : __TIME__ )
  {
    shifted <<= 8;
    shifted |= c;
  }

  return shifted;
}

struct PCG
{
  struct pcg32_random_t { std::uint64_t state=0;  std::uint64_t inc=seed(); };
  pcg32_random_t rng;
  typedef std::uint32_t result_type;

  constexpr result_type operator()()
  {
    return pcg32_random_r();
  }

  static result_type constexpr min()
  {
    return std::numeric_limits<result_type>::min();
  }

  static result_type constexpr max()
  {
    return std::numeric_limits<result_type>::min();
  }

  private:
  constexpr std::uint32_t pcg32_random_r()
  {
    std::uint64_t oldstate = rng.state;
    // Advance internal state
    rng.state = oldstate * 6364136223846793005ULL + (rng.inc|1);
    // Calculate output function (XSH RR), uses old state for max ILP
    std::uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    std::uint32_t rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
  }

};

constexpr auto get_random(int count)
{
  PCG pcg;
  while(count > 0){
    pcg();
    --count;
  }
  
  return pcg();
}

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
} // namespace cmap

template<std::size_t TSize>
constexpr auto random() {
  PCG pcg;
  std::array<int, TSize> numbers{};
  for(std::size_t i = 0 ; i < TSize ; ++i) {
    numbers[i] = pcg();
  }
  return numbers;
}


template<std::size_t TSize, std::size_t TDepth = 0>
constexpr auto generate_random_lookup(const std::array<int,TSize>& keys, const std::array<int,TSize>& values) {
  if constexpr (TSize == TDepth) {
    cmap::index<int,int> root;
    return root;
  }
  else {
    return generate_random_lookup<TSize,TDepth + 1>(keys, values).insert(keys[idx], values[idx]);
  }
}



/*
int main() {
  constexpr index<int,int> i0;
  constexpr auto i1 = i0.insert(5,12).insert(6,13).insert(2,17);
  volatile auto key = 5;
  const auto val = i1.find(key, binary_search<int>);
  return val;
}
*/

//constexpr auto make_big_lookup() {
//}

NONIUS_BENCHMARK("random access (constexpr", []{
    constexpr auto keys   = random<100>();
    constexpr auto values = random<100>();
    constexpr auto lookup = generate_random_lookup(keys,values);
    //constexpr auto lookup = make_big_lookup();
})

NONIUS_BENCHMARK("to_string(42)", []{
    return std::to_string(42);
})
