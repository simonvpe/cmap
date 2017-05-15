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

namespace cmap {

constexpr auto map(auto key, auto value) {
  return [key,value](auto _key) {
    return std::pair(_key == key, value);
  };
}

// There is really no need for operator overloading here, it's just that I have
// no idea of how to do the fold expression in `make_map()` with a "regular" function
constexpr auto operator<<(auto left, auto right) {
  return [left,right](auto key) {
      if(const auto [success, value] = left(key); success) {
        return std::pair(true, value);
      }
      if(const auto [success, value] = right(key); success) {
        return std::pair(true, value);
      }
      using T = decltype(left(key).second);
      return std::pair(false, T{});
  };
}

template<typename T>
struct root {
  constexpr root(T _chain) : chain(_chain) {}
  
  constexpr auto operator[](auto key) const {
    const auto [success, value]  = chain(key);
    return success ? value : throw std::out_of_range("No such key!");
  }

  const T chain;
};

template<typename...Ts>
constexpr auto make_map(Ts...ts) {
  const auto chain = (... << ts);
  return root(chain);
}

constexpr auto join(root<auto> left, root<auto> right) {
  return root(left.chain << right.chain);
}
  
} // namespace cmap
