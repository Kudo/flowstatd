cmake_minimum_required(VERSION 2.8)
project(FLOWSTATD)

set(CMAKE_C_FLAGS "-Wall")
set(CMAKE_INSTALL_PREFIX "${FLOWSTATD_SOURCE_DIR}")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif(NOT CMAKE_BUILD_TYPE)

include_directories(${CMAKE_BINARY_DIR}/include)
link_directories(${CMAKE_BINARY_DIR}/lib)

add_subdirectory(src)

include(ExternalProject)
ExternalProject_Add(jansson
    PREFIX jansson
    GIT_REPOSITORY git://github.com/paulharris/jansson.git
    INSTALL_DIR ${CMAKE_BINARY_DIR}
    CMAKE_ARGS ${jansson_CMAKE_ARGS} -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
)

ExternalProject_Add(liblogger
    PREFIX logger
    GIT_REPOSITORY git://github.com/Kudo/liblogger.git
    GIT_TAG runtimeDebugLevel
    INSTALL_DIR ${CMAKE_BINARY_DIR}
    CMAKE_ARGS ${liblogger_CMAKE_ARGS} -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
)
