###############################################################################
# compiler flags for this project
###############################################################################

if (${CMAKE_C_COMPILER_ID} STREQUAL "GNU")
    add_definitions (-W -Wall -Wextra -pedantic -Wno-unused-parameter)
elseif (${CMAKE_C_COMPILER_ID} STREQUAL "Clang")
    add_definitions (-W -Wall -Wextra -pedantic -Wno-unused-parameter)
elseif (${CMAKE_C_COMPILER_ID} STREQUAL "Intel")
elseif (${CMAKE_C_COMPILER_ID} STREQUAL "MSVC")
endif ()

###############################################################################
# source files and runtime definition
###############################################################################

file (GLOB clither_HEADERS "include/clither/*.h")
file (GLOB clither_SOURCES "src/*.c")

set (clither_HEADERS ${clither_HEADERS}
    "include/clither/config.h.in"
)

include_directories ("include")

add_executable (clither
    ${clither_HEADERS}
    ${clither_SOURCES}
)

if(${PLATFORM} MATCHES "LINUX" OR ${PLATFORM} MATCHES "MACOSX")
    set (MATH_LIB m)
endif ()

target_link_libraries (clither
    ${MATH_LIB}
    util
)

# generates the correct project files for visual studio, setting the correct
# debug working directory and other stuff
create_vcproj_userfile (clither)

###############################################################################
# generate config.h and export.h
###############################################################################

set (PROJECT_NAME "CLITHER")

configure_file ("include/clither/config.h.in"
                "include/clither/config.h")

include_directories ("${CMAKE_CURRENT_BINARY_DIR}/include")

###############################################################################
# install targets
###############################################################################

install (
    TARGETS
        clither
    DESTINATION
        "bin"
)
install (
    DIRECTORY
        "${CMAKE_CURRENT_LIST_DIR}/cfg"
    DESTINATION
        "bin"
)

