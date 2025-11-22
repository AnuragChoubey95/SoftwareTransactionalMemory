# --------------------------------------------------------------------
# Build libbloom using its own Makefile (not CMake)
# --------------------------------------------------------------------
add_custom_target(libbloom_build ALL
    COMMAND make
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Building libbloom via its Makefile"
)

# --------------------------------------------------------------------
# Declare libbloom as an IMPORTED static library
# CMake does NOT build this library; Makefile does.
# --------------------------------------------------------------------
add_library(libbloom STATIC IMPORTED GLOBAL)

# --------------------------------------------------------------------
# The path to the static library produced by the Makefile
# --------------------------------------------------------------------
set_target_properties(libbloom PROPERTIES
    IMPORTED_LOCATION
        ${CMAKE_CURRENT_SOURCE_DIR}/build/libbloom.a
)

# --------------------------------------------------------------------
# Ensure that libbloom_build runs BEFORE libbloom is used anywhere
# --------------------------------------------------------------------
add_dependencies(libbloom libbloom_build)

# --------------------------------------------------------------------
# Export header include paths to consumers of this library
# --------------------------------------------------------------------
target_include_directories(libbloom INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/murmur2
)
