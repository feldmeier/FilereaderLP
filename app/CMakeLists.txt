# create readerlp binary using library without pic
add_executable(readerlp run.cpp)
target_link_libraries(readerlp libreaderlp)

# install the binary
install(TARGETS readerlp EXPORT readerlp-targets
        RUNTIME DESTINATION bin)