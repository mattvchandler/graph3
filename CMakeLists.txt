cmake_minimum_required (VERSION 2.6)
project(graph3)

set(CMAKE_CXX_FLAGS "-Wall -std=c++11")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR})

find_package(PkgConfig)
pkg_check_modules(GTKMM gtkmm-3.0)
find_package(SFML 2 COMPONENTS window system)
pkg_check_modules(MUPARSER muparser)
find_package(LibPNG++)
find_package(OpenGL)
find_package(GLEW)
find_package(GLM)

include_directories(${GTKMM_INCLUDE_DIRS}, ${LIBPNG++_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/src/)
link_directories(${GTKMM_LINK_DIRS})

add_executable(${PROJECT_NAME}
    src/graph.cpp
    src/graph_cartesian.cpp
    src/graph_cylindrical.cpp
    src/graph_disp.cpp
    src/graph_window.cpp
    src/main.cpp
    src/SFMLWidget/SFMLWidget.cpp)
target_link_libraries(${PROJECT_NAME}
    ${GTKMM_LIBRARIES} ${SFML_LIBRARIES} ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} ${LIBPNG++_LIBRARIES} ${MUPARSER_LIBRARIES})

