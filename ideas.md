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
#include <tuple>
#include <stdexcept>

namespace cmap {

/*
  The underlying model for the binary tree
 */
namespace _model {
  /*
    A map is built like a binary tree as illustrated below.

            f1(k)
             / \
            /   \
         f2(k)  f3(k)
                 / \
                /   \
             f4(k)  f5(k)
              
    Each subtree f1,f2,f3,f4,f5 is a function which
    evaluates a key and returns an `outcome` which contains 
    information about whether the subtree contained the key,
    and in that case what the associated value was.

    To construct the tree we utilize two factory functions

    * One called `make_terminal(k,v)` which creates a function that
      evaluates a key and returns an whether it matches a particular
      constant and the associated value.

    * One called `make_branch(left,right)` which creates a branch 
      node, a function that first evaluats and returns the result of 
      evaluating the left subtree if successful. If unsuccessful it 
      evaluates the right subtree and returns the result of the 
      evaluation of the right subtree.

    Example: Construct the tree above

      const auto f5 = make_terminal(5,15);
      const auto f4 = make_terminal(4,14);
      const auto f2 = make_terminal(3,13);
      const auto f3 = make_branch(f4, f5);
      const auto f1 = make_branch(f2, f3);

      // Performing a lookup for the value `5` would
      // produce the stacktrace
      f1(5)
        f2(5) -> {false, 13}
        f3(5)
          f4(5) -> {false, 14}
          f5(5) -> {true, 15}
        ...
      -> {true, 15}

    In order to easily chain together multiple subtrees there is a
    utility function called `merge(node1, ...)` which takes all the
    terminal nodes as arguments and automatically creates the branches. 

    To reproduce the previous example using the merge function one could do 
    the following.

    Example: Construct the same tree using the `merge` function

      const auto f5 = make_terminal(5,15);
      const auto f4 = make_terminal(4,14);
      const auto f2 = make_terminal(3,13);
      const auto f1 = merge(f2,f4,f5);

    Since the whole model is completely independent of the datatype stored
    there is literally no limit to what you can store in the map, as long
    as it is copy constructible. That means that you can nest maps and store
    complex types.
  */
  using std::pair;

  template<typename V> struct outcome {
    constexpr outcome(bool s, const V v) : success{s}, value{v} {}
    const bool success;
    const V value;
  };

  template<typename K, typename V>
  constexpr auto make_terminal(const K key, const V value) {
    return [key,value](const auto _key) {
      return outcome{_key == key, value};
    };
  };

  constexpr auto make_branch(const auto left, const auto right) {
    return [left,right](auto key) {
      if(const auto [result, value] = left(key); result) {
        return outcome{true, value};
      }
      return right(key);
    };
  }

  constexpr auto merge(const auto node) {
    return node;
  }

  constexpr auto merge(const auto left, const auto ... rest) {
    return make_branch(left, merge(rest...));
  }

}

/*
  Functional interface

  Example:
    constexpr auto map = make_map(map(13,43), map(14,44));
    constexpr auto fourty_three = lookup(map, 13);
    constexpr auto fourty_four  = lookup(map, 14);
 */

constexpr auto make_map(const auto ... rest) {
  return _model::merge(rest...);
}

constexpr auto map(const auto key, const auto value) {
  return _model::make_terminal(key, value);
}

constexpr auto join(const auto left_map, const auto right_map) {
  return _model::merge(left_map, right_map);
}

constexpr auto lookup(const auto tree, const auto key) {
  const auto [success, value] = tree(key);
  return success ? value : throw std::out_of_range("No such key");
}

/*
  Class interface

  Example:
    constexpr auto map = make_lookup(map(13,43), map(14,44));
    constexpr auto fourty_three = map[13];
    constexpr auto fourty_four  = map[14];
 */

template<typename TLookup>
struct lookup_type {
  constexpr lookup_type(const TLookup m) : map{m} {}
  constexpr auto operator[](const auto key) const { return lookup(map, key); }
  const TLookup map;
};

constexpr auto make_lookup(const auto ... rest) {
  return lookup_type{make_map(rest...)};
}

// BSP Example

enum class Device   { MK65F18 };

//    GPIO MODULE
namespace GPIO {
  enum class Port { PTA, PTB };
  enum class Register { PDOR, PSOR, PCOR, PTOR, PDIR, PDDR };

  constexpr auto Module = make_lookup(
    map(Device::MK65F18, make_lookup(
      map(Port::PTA, make_lookup(
        map(Register::PDOR, 0x400FF000u),
        map(Register::PSOR, 0x400FF004u),
        map(Register::PCOR, 0x400FF008u),
        map(Register::PTOR, 0x400FF00Cu),
        map(Register::PDIR, 0x400FF00Eu),
        map(Register::PDDR, 0x400FF010u)
      )),
      map(Port::PTB, make_lookup( 
        map(Register::PDOR, 0x400FF040u),
        map(Register::PSOR, 0x400FF044u),
        map(Register::PCOR, 0x400FF048u),
        map(Register::PTOR, 0x400FF04Cu),
        map(Register::PDIR, 0x400FF04Eu),
        map(Register::PDDR, 0x400FF050u)
      ))
    ))
  );
}

//    I2C MODULE
namespace I2C {
  enum class Port { I2C0, I2C1, I2C2, I2C3 };
  enum class Register { A1, F, C1, S, D, C2, FLT, RA, SMB, A2, SLTH, SLTL };

  constexpr auto Module = make_lookup(
    map(Device::MK65F18, make_lookup(
      map(Port::I2C0, make_lookup(
        map(Register::A1, 0x40066000u)
      )),
      map(Port::I2C1, make_lookup(
        map(Register::A1, 0x40067000u)
      )),
      map(Port::I2C2, make_lookup(
        map(Register::A1, 0x400E6000u)
      )),
      map(Port::I2C3, make_lookup(
        map(Register::A1, 0x400E7000u)
      ))      
    ))
  );
}

//    BSP
template<Device TDevice> struct bsp {
  struct gpio {
    static constexpr auto write = bsp::mk_write_fn(GPIO::Module);
    static constexpr auto read = bsp::mk_read_fn(GPIO::Module);
    static constexpr auto rmw = bsp::mk_rmw_fn(GPIO::Module);
  };
  struct i2c {
    static constexpr auto write = bsp::mk_write_fn(I2C::Module);
    static constexpr auto read = bsp::mk_read_fn(I2C::Module);
    static constexpr auto rmw = bsp::mk_rmw_fn(I2C::Module);
  };

  private:
  static constexpr auto mk_read_fn(auto module) { 
    return [module](auto port, auto reg) {
      return *reinterpret_cast<volatile const uint32_t*>(module[TDevice][port][reg]); 
    };
  };

  static constexpr auto mk_write_fn(auto module) { 
    return [module](auto port, auto reg, uint32_t value) {
      *reinterpret_cast<volatile uint32_t*>(module[TDevice][port][reg]) = value; 
    };
  };

  static constexpr auto mk_rmw_fn(auto module) {
    const auto read = mk_read_fn(module);
    const auto write = mk_write_fn(module);
    return [module, read, write](auto port, auto reg, auto fn) {
      const auto old_value = read(port, reg);
      const auto new_value = fn(old_value);
      write(port, reg, new_value);
      return new_value;
    };
  }
};

// Using the BSP
auto foobar() {
  using BOARD = bsp<Device::MK65F18>;
  BOARD::i2c::rmw(I2C::Port::I2C0, I2C::Register::A1, [](auto value) {
    return value | 0xf;
  });
  return BOARD::i2c::read(I2C::Port::I2C0, I2C::Register::A1);
}

// Verification

auto foo() {
  constexpr auto map0 = make_map(map(13,43),map(14,44));
  constexpr auto map1 = make_map(map(12,42),map(15,45));
  constexpr auto map3 = make_map(map(0,map0), map(1,map1));
  constexpr auto map4 = join(map0,map1);

  static_assert(lookup(lookup(map3, 0), 13) == 43);

  static_assert(lookup(map0, 13) == 43);

  static_assert(lookup(map4, 13) == 43);
  static_assert(lookup(map4, 12) == 42);

  constexpr auto look0 = make_lookup(map(13,43));
  static_assert(look0[13] == 43);

  constexpr auto look1 = make_lookup(map("HELLO",10), map("WORLD",20));
  constexpr auto look2 = make_lookup(map("HELLO",11), map("WORLD",21));
  constexpr auto look3 = make_lookup(map("A",look1),map("B",look2));
  static_assert(look3["A"]["HELLO"] == 10);

  return lookup(map0,14);
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
