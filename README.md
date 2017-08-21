[![Build Status](http://ec2-54-245-163-75.us-west-2.compute.amazonaws.com/api/badges/simonvpe/cmap/status.svg)](http://ec2-54-245-163-75.us-west-2.compute.amazonaws.com/simonvpe/cmap)
# A C++17 compile time lookup table single header library
This library solves static mapping of keys to values in compile time, allowing for highly optimized code, yet short and concise.

## Example ##
### Basic usage ###

```c++
#include <cmap.hpp>

using cmap::make_map;
using cmap::map;

// Mapping of int -> int
constexpr auto lookup       = make_map(map(12,42), map(13,43));
constexpr auto fourty_two   = lookup[12];
constexpr auto fourty_three = lookup[13];

// Trying to access a bad index will trigger a compiler error
constexpr auto compiler_error = lookup[14];
// error: expression ‘<throw-expression>’ is not a constant expression

// On the other hand, using non constexpr variables an exception is thrown instead
const auto exception_thrown = lookup[14];
// throws std::out_of_range

// Do whatever you want with the lookup result
char data[ lookup[12]; // char data[42];
```
### Custom types ###
```c++
#include <cmap.hpp>

using cmap::make_map;
using cmap::map;

struct mytype {
  // Equality comparison operator required!
  constexpr bool operator==(const mytype& other) const {
    return value == other.value;
  }
  const int value;
};

constexpr auto lookup_a     = make_map(map(mytype{12},42), map(mytype{13},43));
constexpr auto fourty_two   = lookup_a[mytype{12}];
constexpr auto fourty_three = lookup_a[mytype{13}];

constexpr auto lookup_b     = make_map(map(42,mytype{12}), map(43,mytype{13}));
constexpr mytype twelve     = lookup_b[42];
constexpr mytype thirteen   = lookup_b[43];
```

### Joining several maps ###
```c++
#include <cmap.hpp>

using cmap::make_map;
using cmap::map;
using cmap::join;

constexpr auto lookup_one = make_map(map(12,42), map(13,43));
constexpr auto lookup_two = make_map(map(14,44), map(15,45));
constexpr auto lookup     = join(lookup_one, lookup_two);

constexpr auto fourty_two   = lookup[12];
constexpr auto fourty_three = lookup[13];
constexpr auto fourty_four  = lookup[14];
constexpr auto fourty_five  = lookup[15];
```

### Nested maps ###
```c++
#include <cmap.hpp>

using cmap::make_map;
using cmap::map;

constexpr auto lookup = make_map(
  map('A', make_map(
    map(12, 42),
    map(13, 43)
  )),
  map('B', make_map(
    map(14, 44),
    map(15, 45)
  ))
);

constexpr auto fourty_two   = lookup['A'][12];
constexpr auto fourty_three = lookup['A'][13];
constexpr auto fourty_four  = lookup['B'][14];
constexpr auto fourty_five  = lookup['B'][15];
```

## Installation ##
Simply `#include <cmap.hpp>`.

## Running the tests ##
Make sure you have a C++17 compiler (preferably gcc 7) and cmake >= 3.7 installed. Additionally, for the performance tests you'll need Boost.System.
```bash
$ git clone https://github.com/simonvpe/cmap.git
$ cd cmap && mkdir build && cd build && cmake .. && make && ./test/tests
```

## Built With ##
* gcc 7.0.1
* cmake 3.8

## Authors ##
* Simon Pettersson

## License ##
This project is licensed under the MIT License - see the LICENSE file for details
