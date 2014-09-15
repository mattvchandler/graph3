cmake_minimum_required (VERSION 2.6)
project(graph3)
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)
set(GRAPH_TITLE "Graph 3")
set(GRAPH_AUTHOR "Matthew Chandler <tardarsauce@gmail.com>")
set(GRAPH_SUMMARY "A 3D graphing tool")
set(GRAPH_WEBSITE "http://github.com/mattvchandler/graph3")

#flags
set(CMAKE_CXX_FLAGS "-Wall -std=c++14")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG")
if(MSYS)
    set(CMAKE_EXE_LINKER_FLAGS "-mwindows")
endif()
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

# libraries
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR})

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM gtkmm-3.0 REQUIRED REQUIRED)
find_package(SFML 2 COMPONENTS window system REQUIRED)
pkg_check_modules(MUPARSER muparser REQUIRED)
pkg_check_modules(LIBCONFIG libconfig++ REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLM REQUIRED)

# configure variables
set(bindir ${CMAKE_INSTALL_PREFIX}/bin)
set(datadir ${CMAKE_INSTALL_PREFIX}/share/graph3)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/config.hpp.in
    ${PROJECT_BINARY_DIR}/src/config.hpp)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/graph3.desktop.in
    ${PROJECT_BINARY_DIR}/graph3.desktop)

# directories
include_directories(${GTKMM_INCLUDE_DIRS}
    ${LIBCONFIG_INCLUDE_DIRS}
    ${PROJECT_BINARY_DIR}/src/
    ${CMAKE_CURRENT_SOURCE_DIR}/src/)
link_directories(${GTKMM_LINK_DIRS}
    ${LIBCONFIG_LINK_DIRS})

# main compilation
add_executable(${PROJECT_NAME}
    src/config.cpp
    src/gl_helpers.cpp
    src/graph.cpp
    src/graph_cartesian.cpp
    src/graph_cylindrical.cpp
    src/graph_disp.cpp
    src/graph_page.cpp
    src/graph_parametric.cpp
    src/graph_spherical.cpp
    src/graph_window.cpp
    src/lighting_window.cpp
    src/main.cpp
    src/SFMLWidget/SFMLWidget.cpp
    src/tab_label.cpp)

target_link_libraries(${PROJECT_NAME}
    ${GTKMM_LIBRARIES}
    ${SFML_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${MUPARSER_LIBRARIES}
    ${LIBCONFIG_LIBRARIES})

# install targets
install(TARGETS "${PROJECT_NAME}" DESTINATION "bin")
install(FILES "img/cursor.png" DESTINATION "share/graph3/img")
install(FILES "${PROJECT_BINARY_DIR}/graph3.desktop" DESTINATION "share/applications")
install(DIRECTORY "shaders" "examples" "icons" DESTINATION "share/graph3")

# CPack vars
set(CPACK_GENERATOR TGZ)
set(CPACK_PACKAGE_CONTACT "${GRAPH_AUTHOR}")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY)
set(CPACK_INSTALL_CMAKE_PROJECTS
    "${CMAKE_BINARY_DIR};${PROJECT_NAME};ALL;/")
set(CPACK_PACKAGE_VERSION_MAJOR "${VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${VERSION_PATCH}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${GRAPH_TITLE}: ${GRAPH_SUMMARY}")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README")
set(CPACK_PACKAGE_FILE_README "${CMAKE_SOURCE_DIR}/README")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README")
set(CPACK_PACKAGE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_EXECUTABLES "graph3")

# Debian-specific settings
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    find_program(DPKG "dpkg")
    if(DPKG)
        set(CPACK_GENERATOR DEB;TGZ)
        execute_process(COMMAND ${DPKG} --print-architecture
            OUTPUT_STRIP_TRAILING_WHITESPACE
            OUTPUT_VARIABLE CPACK_DEBIAN_PACKAGE_ARCHITECTURE)
        set(CPACK_DEBIAN_PACKAGE_DEPENDS
            "libgtkmm-3.0-1 (>=3.0), libsfml-window2(>=2.0), libsfml-system2(>=2.0), libconfig9, libmuparser2")
        set(CPACK_DEBIAN_PACKAGE_SECTION "education")
        string(TOLOWER "${PROJECT_NAME}" PACKAGE_NAME_LOWER)
        set(CPACK_PACKAGE_FILE_NAME
            "${PACKAGE_NAME_LOWER}_${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")
    endif()
endif()

# Windows NSIS specifics:
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(CPACK_GENERATOR NSIS;TGZ)
    set(CPACK_PACKAGE_EXECUTABLES "graph3;Graph 3")
    set(CPACK_NSIS_DISPLAY_NAME "Graph 3")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "graph3")
    set(CPACK_NSIS_CONTACT "${GRAPH_AUTHOR}")
    set(CPACK_NSIS_HELP_LINK "${GRAPH_WEBSITE}")
    set(CPACK_NSIS_INSTALLED_ICON_NAME "icons\icon_128.ico")
endif()

include(CPack)
