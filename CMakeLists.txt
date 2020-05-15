cmake_minimum_required(VERSION 3.14.3)

project(michal_vs_computers LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#----------------------
#   discount
#----------------------

set(DISCOUNT_ONLY_LIBRARY ON FORCE)
add_subdirectory(${PROJECT_SOURCE_DIR}/discount/cmake)

#----------------------
#   simple web server
#----------------------

set(USE_OPENSSL OFF FORCE)
add_subdirectory(${PROJECT_SOURCE_DIR}/Simple-Web-Server)

#----------------------
#   Web Server
#----------------------
file(GLOB PROJ_SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp ${PROJECT_SOURCE_DIR}/src/**/*.cpp)

add_executable(MichalVSComputers)
target_include_directories(MichalVSComputers PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_sources(MichalVSComputers PRIVATE ${PROJ_SOURCES})
target_link_libraries(MichalVSComputers PRIVATE
  libmarkdown # DISCOUNT
  simple-web-server
  )
set_target_properties(MichalVSComputers PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build")
file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/build/Resources)
add_custom_command(
  TARGET MichalVSComputers POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${PROJECT_SOURCE_DIR}/Resources
  ${PROJECT_SOURCE_DIR}/build/Resources
  )
