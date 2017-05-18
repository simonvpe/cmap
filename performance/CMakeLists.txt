enable_testing()

# Prepare "Catch" library for other executables
set(NONIUS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/nonius/include)
set(MR_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../include)

add_library(nonius INTERFACE)

target_include_directories(nonius INTERFACE ${NONIUS_INCLUDE_DIR})
target_include_directories(nonius INTERFACE ${MR_INCLUDE_DIR})

# Make test executable
set(NONIUS_SOURCES 
	${CMAKE_CURRENT_SOURCE_DIR}/perfrunner.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/cmap.cpp	
)

find_package(Boost REQUIRED COMPONENTS system)
if(Boost_FOUND)
#  include_directories    (${Boost_INCLUDE_DIRS})
  add_executable         (performance ${NONIUS_SOURCES})
  target_link_libraries  (performance nonius)
  target_compile_features(performance PRIVATE cxx_std_17)

  set(THREADS_PREFER_PTHREAD_FLAG ON)
  find_package(Threads REQUIRED)
  target_link_libraries(performance Threads::Threads)
endif()
