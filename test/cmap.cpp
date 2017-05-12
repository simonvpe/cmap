#include <catch.hpp>
#include <cmap.hpp>

using cmap::make_map;
using cmap::map;

SCENARIO("Helo") {
  constexpr auto lookup = make_map(map(1,2), map(2,3), map(5,7));
  CHECK( lookup[1] == 2 );
  CHECK( lookup[2] == 3 );
  CHECK( lookup[5] == 7 );  
}
