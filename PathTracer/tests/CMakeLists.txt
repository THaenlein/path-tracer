set(PROJECT_TEST_NAME "${PROJECT_NAME}_test")

find_package(GTest QUIET)

if(TARGET GTest::gtest)
  set(PATH_TRACER_GTEST_TARGET GTest::gtest)
elseif(EXISTS "${PROJECT_SOURCE_DIR}/lib/googletest/src/gtest-all.cc")
  add_library(pathtracer_gtest STATIC
    "${PROJECT_SOURCE_DIR}/lib/googletest/src/gtest-all.cc"
  )
  target_include_directories(pathtracer_gtest
    PUBLIC
      "${PROJECT_SOURCE_DIR}/lib/googletest/include"
    PRIVATE
      "${PROJECT_SOURCE_DIR}/lib/googletest"
  )
  set(PATH_TRACER_GTEST_TARGET pathtracer_gtest)
elseif(EXISTS "${PROJECT_SOURCE_DIR}/lib/gtest.lib")
  add_library(pathtracer_gtest UNKNOWN IMPORTED)
  set_target_properties(pathtracer_gtest PROPERTIES
    IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/lib/gtest.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${PROJECT_SOURCE_DIR}/lib/googletest/include"
  )
  set(PATH_TRACER_GTEST_TARGET pathtracer_gtest)
else()
  find_package(GTest REQUIRED)
  set(PATH_TRACER_GTEST_TARGET GTest::gtest)
endif()

add_executable("${PROJECT_TEST_NAME}"
  "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/TestRayTracer.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/TestBoundingBox.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/TestRayTracer.hpp"
  "${CMAKE_CURRENT_LIST_DIR}/TestBoundingBox.hpp"
)

target_link_libraries("${PROJECT_TEST_NAME}" PRIVATE ${PATH_TRACER_GTEST_TARGET})
target_link_libraries("${PROJECT_TEST_NAME}" PRIVATE PathTracer_lib)
target_compile_definitions("${PROJECT_TEST_NAME}" PRIVATE PATH_TRACER_PROJECT_DIR="${PROJECT_SOURCE_DIR}")

add_test(NAME "${PROJECT_TEST_NAME}" COMMAND "${PROJECT_TEST_NAME}")

if(WIN32)
  add_custom_command(TARGET "${PROJECT_TEST_NAME}" POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
      "${PROJECT_SOURCE_DIR}/dll"
      "$<TARGET_FILE_DIR:${PROJECT_TEST_NAME}>"
  )
endif()
