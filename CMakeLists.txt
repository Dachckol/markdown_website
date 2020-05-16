cmake_minimum_required(VERSION 3.14.3)

project(markdown_website LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#----------------------
#   discount
#----------------------

add_library(libmarkdown IMPORTED STATIC)
set_target_properties(libmarkdown PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${PROJECT_SOURCE_DIR}/discount
  IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/discount/libmarkdown.a
  )

#----------------------
#   httplib
#----------------------

add_library(httplib INTERFACE)
target_link_libraries(httplib INTERFACE pthread)
target_include_directories(httplib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/cpp-httplib)

#----------------------
#   Web Server
#----------------------
file(GLOB PROJ_SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp ${PROJECT_SOURCE_DIR}/src/**/*.cpp)

add_executable(MarkdownWebsite)
target_include_directories(MarkdownWebsite PRIVATE
  ${PROJECT_SOURCE_DIR}/src
  )
target_sources(MarkdownWebsite PRIVATE ${PROJ_SOURCES})
target_link_libraries(MarkdownWebsite PRIVATE
  libmarkdown # DISCOUNT
  httplib
  )
set_target_properties(MarkdownWebsite PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build")
file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/build/Resources)
add_custom_command(
  TARGET MarkdownWebsite POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${PROJECT_SOURCE_DIR}/Resources
  ${PROJECT_SOURCE_DIR}/build/Resources
  )
