namespace cmap {

template<typename TKey, typename TValue>
constexpr auto map(TKey key, TValue value) {
  return [key,value](TKey _key) {
    return std::pair(_key == key, value);
  };
}

constexpr auto operator<<(auto left, auto right) {
  return [left,right](auto key) {
      const auto [lresult, lvalue] = left(key);
      if(lresult) {
        return std::pair(true, lvalue);
      }
      const auto [rresult, rvalue] = right(key);
      return std::pair(rresult, rvalue);
  };
}

template<typename T>
class root {
  public:
  constexpr root(T _chain) : chain(_chain) {}
  constexpr auto operator[](auto key) const {
    const auto lookup  = chain(key);
    const auto success = lookup.first;
    const auto value   = lookup.second;
    return success ? value : throw std::out_of_range("No such key!");
  }

  private:
  const T chain;
};

template<typename...Ts>
constexpr auto make_map(Ts...ts) {
  const auto chain = (... << ts);
  return root(chain);
}

} // namespace cmap
