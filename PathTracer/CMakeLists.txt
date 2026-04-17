cmake_minimum_required(VERSION 3.16)
project(PathTracer LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(CTest)

option(PATH_TRACER_BUILD_TESTS "Build PathTracer tests" ON)
option(PATH_TRACER_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)

function(path_tracer_enable_warnings target_name)
  if(MSVC)
    target_compile_options(${target_name} PRIVATE /W4 /permissive-)
    if(PATH_TRACER_WARNINGS_AS_ERRORS)
      target_compile_options(${target_name} PRIVATE /WX)
    endif()
  else()
    target_compile_options(${target_name} PRIVATE -Wall -Wextra -Wpedantic)
    if(PATH_TRACER_WARNINGS_AS_ERRORS)
      target_compile_options(${target_name} PRIVATE -Werror)
    endif()
  endif()
endfunction()

file(GLOB_RECURSE PROJECT_SOURCE_FILES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

file(GLOB_RECURSE PROJECT_HEADER_FILES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
)

list(REMOVE_ITEM PROJECT_SOURCE_FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/template.cpp"
)
list(REMOVE_ITEM PROJECT_HEADER_FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/src/main.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/template.hpp"
)

find_package(SDL2 REQUIRED)
find_package(ASSIMP REQUIRED)

add_library(STB_IMAGE STATIC
  "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/stb_image.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/stb_image_write.cpp"
)
target_include_directories(STB_IMAGE PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_library(PathTracer_lib STATIC ${PROJECT_SOURCE_FILES} ${PROJECT_HEADER_FILES})
target_include_directories(PathTracer_lib
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_link_libraries(PathTracer_lib
  PUBLIC
    STB_IMAGE
    ${SDL2_LIBRARIES}
    ${ASSIMP_LIBRARIES}
)
path_tracer_enable_warnings(PathTracer_lib)

add_executable(PathTracer "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
target_link_libraries(PathTracer PRIVATE PathTracer_lib)
path_tracer_enable_warnings(PathTracer)

if(WIN32)
  add_custom_command(TARGET PathTracer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
      "${PROJECT_SOURCE_DIR}/dll"
      "$<TARGET_FILE_DIR:PathTracer>"
  )
  set_property(DIRECTORY ${CMAKE_CURRENT_LIST_DIR} PROPERTY VS_STARTUP_PROJECT PathTracer)
endif()

if(PATH_TRACER_BUILD_TESTS)
  add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/tests")
endif()
