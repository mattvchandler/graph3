# try to find libpng++

# defines:
#   LIBPNG++_FOUND - if found
#   LIBPNG++_INCLUDE_DIRS
#   LIBPNG++_LIBRARIES
#   LIBPNG++_DEFINITIONS

find_package(PNG)

find_path(LIBPNG++_INCLUDE_DIR png++/png.hpp)
set(LIBPNG++_LIBRARY ${PNG_LIBRARIES})

set(LIBPNG++_INCLUDE_DIRS ${LIBPNG++_INCLUDE_DIR} ${PNG_INCLUDE_DIRS})
set(LIBPNG++_LIBRARIES ${LIBPNG++_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LIBPNG++ DEFAULT_MSG
    LIBPNG++_LIBRARY LIBPNG++_INCLUDE_DIR)
mark_as_advanced(LIBPNG++_INCLUDE_DIR LIBPNG++_LIBRARY)
