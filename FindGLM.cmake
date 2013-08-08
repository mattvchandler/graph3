# try to find GLM

# defines:
#   GLM_FOUND - if found
#   GLM_INCLUDE_DIRS
#   GLM_LIBRARIES - alays blank (no libs needed)
#   GLM_DEFINITIONS - alays blank (no defs needed)

find_path(GLM_INCLUDE_DIR glm/glm.hpp)

set(GLM_INCLUDE_DIRS ${GLM_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLM DEFAULT_MSG
    GLM_INCLUDE_DIR)
mark_as_advanced(GLM_INCLUDE_DIR)
