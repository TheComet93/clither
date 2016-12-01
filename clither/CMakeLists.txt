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

add_executable (clither
    "src/main.c"
    "src/argv.c"
    "include/clither/argv.h")

include_directories ("include")

target_link_libraries (clither util)
target_link_libraries (clither game)

# generates the correct project files for visual studio, setting the correct
# debug working directory and other stuff
create_vcproj_userfile (clither)

###############################################################################
# install targets
###############################################################################

install (
    TARGETS
        clither
    DESTINATION
        "bin"
)

