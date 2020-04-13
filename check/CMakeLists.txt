include(CTest)

# prepare Catch library
set(CATCH_INCLUDE_DIR ${FilereaderLp_DIR}/external/catch)
add_library(Catch INTERFACE)
target_include_directories(Catch INTERFACE ${CATCH_INCLUDE_DIR})
target_include_directories(Catch INTERFACE ${FilereaderLp_SOURCE_DIR}/src)

add_executable(unit_tests test.cpp)
target_link_libraries(unit_tests libreaderlp Catch)
add_test(NAME unit_tests COMMAND unit_tests)
enable_testing()
