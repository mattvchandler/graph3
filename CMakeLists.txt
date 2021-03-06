cmake_minimum_required (VERSION 2.6)
project(graph3)
set(TITLE "Graph 3")
set(AUTHOR "Matthew Chandler <tardarsauce@gmail.com>")
set(COPYRIGHT "2018 Matthew Chandler")
set(SUMMARY "A 3D graphing tool")
set(WEBSITE "http://github.com/mattvchandler/graph3")

#flags
set(CMAKE_CXX_FLAGS "-Wall -std=c++14")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

if(MSYS AND CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_EXE_LINKER_FLAGS "-mwindows")
endif()

# libraries
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM gtkmm-3.0 REQUIRED)
find_package(SFML 2 COMPONENTS window system REQUIRED)
pkg_check_modules(MUPARSER muparser REQUIRED)
pkg_check_modules(LIBCONFIG libconfig++ REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLM REQUIRED)

# configure variables
set(GRAPH3_GIT_VERSIONING ON CACHE INTERNAL "")

set(bindir ${CMAKE_INSTALL_PREFIX}/bin)
set(datadir ${CMAKE_INSTALL_PREFIX}/share/graph3)

# directories
include_directories(${GTKMM_INCLUDE_DIRS}
    ${LIBCONFIG_INCLUDE_DIRS}
    ${PROJECT_BINARY_DIR}/src/
    ${CMAKE_CURRENT_SOURCE_DIR}/src/)
link_directories(${GTKMM_LIBRARY_DIRS}
    ${LIBCONFIG_LIBRARY_DIRS}
    ${MUPARSER_LIBRARY_DIRS})

# git version info
if(GRAPH3_GIT_VERSIONING)
    find_package(Git)
endif()
if(GIT_FOUND)
    file(GENERATE OUTPUT ${PROJECT_BINARY_DIR}/version.cmake CONTENT
        "# copy vars from main CMakeLists.txt
        set(CMAKE_CURRENT_SOURCE_DIR \"${CMAKE_CURRENT_SOURCE_DIR}\")
        set(PROJECT_BINARY_DIR \"${PROJECT_BINARY_DIR}\")
        set(PROJECT_NAME \"${PROJECT_NAME}\")
        set(TITLE \"${TITLE}\")
        set(AUTHOR \"${AUTHOR}\")
        set(COPYRIGHT \"${COPYRIGHT}\")
        set(SUMMARY \"${SUMMARY}\")
        set(WEBSITE \"${WEBSITE}\")
        set(bindir \"${bindir}\")
        set(datadir \"${datadir}\")

        execute_process(COMMAND \"${GIT_EXECUTABLE}\" describe --tags --dirty
            OUTPUT_STRIP_TRAILING_WHITESPACE
            OUTPUT_VARIABLE VERSION_FULL)

        string(REGEX REPLACE \"^v([0-9]+)\\\\..*\" \"\\\\1\" VERSION_MAJOR \"\${VERSION_FULL}\")
        string(REGEX REPLACE \"^v[0-9]+\\\\.([0-9]+).*\" \"\\\\1\" VERSION_MINOR \"\${VERSION_FULL}\")
        string(REGEX REPLACE \"^v[0-9]+\\\\.[0-9]+-([0-9]+).*\" \"\\\\1\" VERSION_PATCH \"\${VERSION_FULL}\")
        string(REGEX REPLACE \"^v[0-9]+\\\\.[0-9]+-[0-9]+-(.*)\" \"\\\\1\" VERSION_SHA1 \"\${VERSION_FULL}\")

        if(NOT VERSION_PATCH OR VERSION_PATCH STREQUAL VERSION_FULL)
            set(VERSION_PATCH 0)
        endif()
        if(NOT VERSION_SHA1 OR VERSION_SHA1 STREQUAL VERSION_FULL)
            set(VERSION_SHA1 \"\")
        endif()

        set(VERSION_SHORT \"\${VERSION_MAJOR}.\${VERSION_MINOR}.\${VERSION_PATCH}\")

        message(STATUS \"Graph3 version \${VERSION_FULL}\")

        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/config.hpp.in
            ${PROJECT_BINARY_DIR}/src/config.hpp)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/graph3.desktop.in
            ${PROJECT_BINARY_DIR}/graph3.desktop)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/graph3.rc.in
            ${PROJECT_BINARY_DIR}/graph3.rc)\n")

    add_custom_target(version
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/version.cmake
        DEPENDS
            ${CMAKE_CURRENT_SOURCE_DIR}/src/config.hpp.in
            ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.desktop.in
            ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.rc.in
        BYPRODUCTS
            ${PROJECT_BINARY_DIR}/src/config.hpp
            ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.desktop
            ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.rc
        COMMENT "Getting version from git"
        )

    execute_process(COMMAND ${GIT_EXECUTABLE} describe --tags --dirty
        OUTPUT_STRIP_TRAILING_WHITESPACE
        OUTPUT_VARIABLE VERSION_FULL)

    string(REGEX REPLACE "^v([0-9]+)\\..*" "\\1" VERSION_MAJOR "${VERSION_FULL}")
    string(REGEX REPLACE "^v[0-9]+\\.([0-9]+).*" "\\1" VERSION_MINOR "${VERSION_FULL}")
    string(REGEX REPLACE "^v[0-9]+\\.[0-9]+-([0-9]+).*" "\\1" VERSION_PATCH "${VERSION_FULL}")
    string(REGEX REPLACE "^v[0-9]+\\.[0-9]+-[0-9]+-(.*)" "\\1" VERSION_SHA1 "${VERSION_FULL}")

    if(NOT VERSION_PATCH OR VERSION_PATCH STREQUAL VERSION_FULL)
        set(VERSION_PATCH 0)
    endif()
    if(NOT VERSION_SHA1 OR VERSION_SHA1 STREQUAL VERSION_FULL)
        set(VERSION_SHA1 "")
    endif()

    set(VERSION_SHORT "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
else()
    # manually update each minor release
    set(VERSION_MAJOR 1)
    set(VERSION_MINOR 5)
    set(VERSION_PATCH 0)
    set(VERSION_SHA1 "")
    set(VERSION_SHORT "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
    set(VERSION_FULL "${VERSION_SHORT}")

    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/config.hpp.in
        ${PROJECT_BINARY_DIR}/src/config.hpp)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.desktop.in
        ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.desktop)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.rc.in
        ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.rc)
endif()
# main compilation
add_executable(${PROJECT_NAME}
    ${PROJECT_BINARY_DIR}/graph3.rc
    src/config.cpp
    src/gl_helpers.cpp
    src/graph_cartesian.cpp
    src/graph.cpp
    src/graph_cylindrical.cpp
    src/graph_disp.cpp
    src/graph_disp_draw.cpp
    src/graph_disp_input.cpp
    src/graph_page_color_tex.cpp
    src/graph_page.cpp
    src/graph_page_file_io.cpp
    src/graph_parametric.cpp
    src/graph_spherical.cpp
    src/graph_util.cpp
    src/graph_window.cpp
    src/image_button.cpp
    src/lighting_window.cpp
    src/main.cpp
    src/SFMLWidget/SFMLWidget.cpp
    src/tab_label.cpp)

if(GIT_FOUND)
    add_dependencies(${PROJECT_NAME} version)
endif()

target_link_libraries(${PROJECT_NAME}
    ${GTKMM_LIBRARIES}
    ${SFML_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${MUPARSER_LIBRARIES}
    ${LIBCONFIG_LIBRARIES})

# install targets
install(TARGETS "${PROJECT_NAME}" DESTINATION "bin")
install(FILES "img/cursor.png" DESTINATION "share/graph3/img")
install(DIRECTORY "shaders" "examples" DESTINATION "share/graph3")

install(FILES "icons/icon.svg" DESTINATION "share/icons/hicolor/scalable/apps" RENAME "graph3.svg")
install(FILES "icons/icon.svg" DESTINATION "share/icons/hicolor/scalable/mimetypes" RENAME "application-x-graph3.svg")
install(FILES "icons/icon_128.png" DESTINATION "share/icons/hicolor/128x128/apps" RENAME "graph3.png")
install(FILES "icons/icon_128.png" DESTINATION "share/icons/hicolor/128x128/mimetypes" RENAME "application-x-graph3.png")
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    install(FILES "${PROJECT_BINARY_DIR}/graph3.desktop" DESTINATION "share/applications")
    install(FILES "x-graph3.xml" DESTINATION "share/mime/packages")
endif()

# CPack vars
set(CPACK_GENERATOR TGZ)
set(CPACK_PACKAGE_CONTACT "${AUTHOR}")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY)
set(CPACK_INSTALL_CMAKE_PROJECTS
    "${CMAKE_BINARY_DIR};${PROJECT_NAME};ALL;/")
set(CPACK_PACKAGE_VERSION_MAJOR "${VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${VERSION_PATCH}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${TITLE}: ${SUMMARY}")
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
            "libgtkmm-3.0-1, libsfml-window2, libsfml-system2, libconfig++9, libmuparser2")
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
    set(CPACK_NSIS_CONTACT "${AUTHOR}")
    set(CPACK_NSIS_HELP_LINK "${WEBSITE}")
    set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/icons/icon_128.ico")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL "ON")
endif()

include(CPack)
