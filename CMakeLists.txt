cmake_minimum_required(VERSION 2.6)
project(FLOWD)

set(CMAKE_C_FLAGS "-Wall")
set(CMAKE_INSTALL_PREFIX "${FLOWD_SOURCE_DIR}")

if(NOT CMAKE_BUILD_TYPE)
  SET( CMAKE_BUILD_TYPE Release)
endif(NOT CMAKE_BUILD_TYPE)
add_subdirectory(src)

