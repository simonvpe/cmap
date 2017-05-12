#include <catch.hpp>
#include <cmap.hpp>

using cmap::make_map;
using cmap::map;

SCENARIO("Compile time mapping") {
  GIVEN("a map of int -> int") {
    constexpr auto lookup = make_map(
        map(1,2),
        map(2,3),
	map(5,7)
    );
    THEN("chek each mapping") {
      CHECK( lookup[1] == 2 );
      CHECK( lookup[2] == 3 );
      CHECK( lookup[5] == 7 );
    }
    THEN("check bad lookup (runtime)") {      
      CHECK_THROWS_AS( lookup[7], std::out_of_range );
    }
    THEN("check bad lookup (compiletime)") {
      // Don't know how to check this, however the following should not compile
      // constexpr auto val = lookup[7];
    }
  }

  GIVEN("a custom type") {
    struct MyType {
      constexpr bool operator==(const MyType& rhs) const { return value == rhs.value; }
      const int value;
    };
    GIVEN("a map of int -> custom type") {
      constexpr auto lookup = make_map(
          map(42, MyType{12}),
          map(43, MyType{13}),
          map(44, MyType{14})
      );
      THEN("check each mapping") {
	CHECK( lookup[42] == MyType{12} );
	CHECK( lookup[43] == MyType{13} );
	CHECK( lookup[44] == MyType{14} );
      }
    }

    GIVEN("a map of custom type -> int") {
      constexpr auto lookup = make_map(
          map(MyType{12}, 42),
          map(MyType{13}, 43),
          map(MyType{14}, 44)
      );
      THEN("check each mapping") {
        CHECK( lookup[MyType{12}] == 42 );
        CHECK( lookup[MyType{13}] == 43 );
        CHECK( lookup[MyType{14}] == 44 );
      }
    }
  }
}
