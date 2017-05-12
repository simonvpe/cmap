enable_testing()

# Prepare "Catch" library for other executables
set(CATCH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Catch/include)
set(MR_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../include)

add_library(Catch INTERFACE)

target_include_directories(Catch INTERFACE ${CATCH_INCLUDE_DIR})
target_include_directories(Catch INTERFACE ${MR_INCLUDE_DIR})

# Make test executable
set(TEST_SOURCES 
	${CMAKE_CURRENT_SOURCE_DIR}/testrunner.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/cmap.cpp	
)
add_executable(tests ${TEST_SOURCES})
target_link_libraries(tests Catch)
target_compile_features(tests PRIVATE cxx_std_17)

add_test(all tests)
