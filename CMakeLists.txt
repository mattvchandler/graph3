cmake_minimum_required (VERSION 2.6)
project(graph3)

set(CMAKE_CXX_FLAGS "-Wall -std=c++11")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR})

find_package(PkgConfig)
pkg_check_modules(GTKMM gtkmm-2.4)
find_package(SFML 2 COMPONENTS window system)
find_package(OpenGL)

include_directories(${GTKMM_INCLUDE_DIRS})
link_directories(${GTKMM_LINK_DIRS})

add_executable(${PROJECT_NAME}
    src/window.cpp
    src/SFML_widget.cpp)
target_link_libraries(${PROJECT_NAME} ${GTKMM_LIBRARIES} ${SFML_LIBRARIES} ${OPENGL_LIBRARIES})
