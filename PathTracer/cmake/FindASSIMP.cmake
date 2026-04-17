include(FindPackageHandleStandardArgs)

find_path(ASSIMP_INCLUDE_DIR
  NAMES assimp/mesh.h
  PATHS
    /usr/include
    /usr/local/include
    /opt/local/include
    "${CMAKE_SOURCE_DIR}/include"
)

find_library(ASSIMP_LIBRARY
  NAMES assimp assimp-vc143-mt
  PATHS
    /usr/lib64
    /usr/lib
    /usr/local/lib
    /opt/local/lib
    "${CMAKE_SOURCE_DIR}/lib"
)

set(ASSIMP_LIBRARIES "${ASSIMP_LIBRARY}")
set(ASSIMP_INCLUDE_DIRS "${ASSIMP_INCLUDE_DIR}")

find_package_handle_standard_args(ASSIMP
  REQUIRED_VARS ASSIMP_LIBRARY ASSIMP_INCLUDE_DIR
)

mark_as_advanced(ASSIMP_INCLUDE_DIR ASSIMP_LIBRARY)
