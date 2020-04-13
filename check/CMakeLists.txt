include_directories(${CMAKE_BINARY_DIR})
configure_file(
    ${CMAKE_SOURCE_DIR}/check/config.hpp.in
    ${CMAKE_BINARY_DIR}/config.hpp
)

# prepare Catch library
set(CATCH_INCLUDE_DIR ${FilereaderLp_DIR}/external/catch)
add_library(Catch INTERFACE)
target_include_directories(Catch INTERFACE ${CATCH_INCLUDE_DIR})
target_include_directories(Catch INTERFACE ${FilereaderLp_SOURCE_DIR}/src)

add_executable(unit_tests test.cpp)
set_property(TARGET unit_tests PROPERTY CXX_STANDARD 11)
target_link_libraries(unit_tests libreaderlp Catch)
add_test(NAME unit_tests COMMAND unit_tests)
enable_testing()
