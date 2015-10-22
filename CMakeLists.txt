cmake_minimum_required(VERSION 2.8)
project(voxel-lidar)

set(EXECUTABLE_NAME voxel-lidar)

# c++14 flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++1y")

# add OGLW library
add_subdirectory(${PROJECT_SOURCE_DIR}/oglw/oglw)

set(WITH_UTILITIES FALSE CACHE BOOL "")
set(WITH_TESTS FALSE CACHE BOOL "")
set(BUILD_OSGEO4W FALSE CACHE BOOL "")
set(WITH_LASZIP TRUE CACHE BOOL "")
add_subdirectory(${PROJECT_SOURCE_DIR}/libLAS)
include_directories(${PROJECT_SOURCE_DIR}/libLAS/include)

# include OGLW headers
include_directories(${OGLW_INCLUDE_DIRS})

# find resources
file(GLOB_RECURSE RESOURCES ${PROJECT_SOURCE_DIR}/resources/*)

# create an executable bundled with resources (OS X)
add_executable(${EXECUTABLE_NAME} MACOSX_BUNDLE src/main.cpp ${RESOURCES})
target_link_libraries(${EXECUTABLE_NAME} OGLW las ${OGLW_LIBRARIES})
set_target_properties(${EXECUTABLE_NAME} PROPERTIES RESOURCE "${RESOURCES}")

