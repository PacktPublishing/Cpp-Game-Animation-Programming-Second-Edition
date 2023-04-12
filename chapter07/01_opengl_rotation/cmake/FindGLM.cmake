#
# Find GLM
#
# Try to find GLM : OpenGL Mathematics.
# This module defines 
# - GLM_INCLUDE_DIRS
# - GLM_FOUND
#
# The following variables can be set as arguments for the module.
# - GLM_ROOT_DIR : Root library directory of GLM 
#
# References:
# - https://github.com/Groovounet/glm/blob/master/util/FindGLM.cmake
# - https://bitbucket.org/alfonse/gltut/src/28636298c1c0/glm-0.9.0.7/FindGLM.cmake
#

# Additional modules
include(FindPackageHandleStandardArgs)

# Find include files
find_path(
	GLM_INCLUDE_DIR
	NAMES glm/glm.hpp
	PATHS
	$ENV{PROGRAMFILES}/include
	$ENV{VK_SDK_PATH}/include
	${GLM_ROOT_DIR}/include
	DOC "The directory where glm/glm.hpp resides")

# Handle REQUIRD argument, define *_FOUND variable
find_package_handle_standard_args(glm DEFAULT_MSG GLM_INCLUDE_DIR)

# Define GLM_INCLUDE_DIRS
if (GLM_FOUND)
	set(GLM_INCLUDE_DIRS ${GLM_INCLUDE_DIR})
endif()

# Hide some variables
mark_as_advanced(GLM_INCLUDE_DIR)

