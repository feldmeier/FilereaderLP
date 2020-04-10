set(sources
   reader.cpp
)

set(headers
   builder.hpp
   model.hpp
   reader.hpp
)

add_library(libreaderlp ${sources})
set_property(TARGET libreaderlp PROPERTY CXX_STANDARD 11)

# install the header files of readerlp
foreach ( file ${headers} )
   get_filename_component( dir ${file} DIRECTORY )
   install( FILES ${file} DESTINATION include/${dir} )
endforeach()

# install the binary and the library to appropriate locations and add them to an export group
install(TARGETS libreaderlp EXPORT readerlp-targets
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
   INCLUDES DESTINATION include)

# Add library targets to the build-tree export set
export(TARGETS libreaderlp
   FILE "${CMAKE_BINARY_DIR}/readerlp-targets.cmake")

#configure the config file for the build tree
#Either list all the src/* directories here, or put explicit paths in all the #include statements
#M reckons that the latter is more transparent, and I'm inclined to agree.
set(CONF_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/src" "${CMAKE_BINARY_DIR}")
configure_file(${CMAKE_SOURCE_DIR}/readerlp-config.cmake.in
   "${CMAKE_BINARY_DIR}/readerlp-config.cmake" @ONLY)

#configure the config file for the install
set(CONF_INCLUDE_DIRS "\${CMAKE_CURRENT_LIST_DIR}/../../../include")
configure_file(${CMAKE_SOURCE_DIR}/readerlp-config.cmake.in
   "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/readerlp-config.cmake" @ONLY)

#configure the pkg-config file for the install
configure_file(${CMAKE_SOURCE_DIR}/readerlp.pc.in
   "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/readerlp.pc" @ONLY)

# install the targets of the readerlp export group, the config file so that other cmake-projects
# can link easily against quareaderlpss, and the pkg-config flie so that other projects can easily
# build against readerlp
install(EXPORT readerlp-targets FILE readerlp-targets.cmake DESTINATION lib/cmake/readerlp)
install(FILES "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/readerlp-config.cmake" DESTINATION lib/cmake/readerlp)
install(FILES "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/readerlp.pc" DESTINATION lib/pkg-config)
