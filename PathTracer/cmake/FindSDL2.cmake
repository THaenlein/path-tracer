include(FindPackageHandleStandardArgs)

find_path(SDL2_INCLUDE_DIR
  NAMES SDL.h
  PATH_SUFFIXES SDL2 sdl2
  PATHS
    /usr/include
    /usr/local/include
    /opt/local/include
    "${CMAKE_SOURCE_DIR}/include"
)

find_library(SDL2_LIBRARY
  NAMES SDL2
  PATHS
    /usr/lib64
    /usr/lib
    /usr/local/lib
    /opt/local/lib
    "${CMAKE_SOURCE_DIR}/lib"
)

find_library(SDL2_MAIN_LIBRARY
  NAMES SDL2main
  PATHS
    /usr/lib64
    /usr/lib
    /usr/local/lib
    /opt/local/lib
    "${CMAKE_SOURCE_DIR}/lib"
)

set(SDL2_LIBRARIES "${SDL2_LIBRARY}")
if(SDL2_MAIN_LIBRARY)
  list(APPEND SDL2_LIBRARIES "${SDL2_MAIN_LIBRARY}")
endif()

set(SDL2_INCLUDE_DIRS "${SDL2_INCLUDE_DIR}")

find_package_handle_standard_args(SDL2
  REQUIRED_VARS SDL2_LIBRARY SDL2_INCLUDE_DIR
)

mark_as_advanced(SDL2_INCLUDE_DIR SDL2_LIBRARY SDL2_MAIN_LIBRARY)
