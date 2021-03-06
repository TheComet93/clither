###############################################################################
# compiler flags for this project
###############################################################################

if (${CMAKE_C_COMPILER_ID} STREQUAL "GNU")
    add_definitions (-W -Wall -Wextra -Werror -pedantic -Wno-unused-parameter)
elseif (${CMAKE_C_COMPILER_ID} STREQUAL "Clang")
    add_definitions (-W -Wall -Wextra -pedantic -Wno-unused-parameter)
elseif (${CMAKE_C_COMPILER_ID} STREQUAL "Intel")
elseif (${CMAKE_C_COMPILER_ID} STREQUAL "MSVC")
endif ()

###############################################################################
# generate config.h for this library
###############################################################################

set (BUILD_TYPE "SHARED")
set (PROJECT_NAME "GAME")

# Set up vulkan libs
set (VULKAN_ARCHITECTURE "x86_64" CACHE STRING "Architecture to use. Defaults to x86_64")
find_path (VULKAN_ROOT "${VULKAN_ARCHITECTURE}/include/vulkan/vulkan.h"
    PATHS "${CMAKE_SOURCE_DIR}/VulkanSDK/linux/1.0.33.0"
          "${CMAKE_SOURCE_DIR}/VulkanSDK/linux/1.0.30.0")
message (STATUS "Found vulkan under ${VULKAN_ROOT}")
if (NOT VULKAN_ROOT)
    message (FATAL_ERROR "Failed to find vulkan")
endif ()
option (VULKAN_USE_DEBUG_MALLOC OFF)

if (NOT VULKAN_CODE_SANITIZED)
    message (STATUS "Replacing C++ style comments in vulkan headers with C style comments")
    set (VULKAN_CODE_SANITIZED ON CACHE STRING "")
    execute_process (COMMAND python3 scripts/cpp2ccomments.py "${VULKAN_ROOT}/${VULKAN_ARCHITECTURE}/include/vulkan/vulkan.h" WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    execute_process (COMMAND python3 scripts/cpp2ccomments.py "${VULKAN_ROOT}/${VULKAN_ARCHITECTURE}/include/vulkan/vk_platform.h" WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endif ()

# print out a summary
message (STATUS "------------------------------------------------------------")
message (STATUS "Settings for game library")
message (STATUS " + Vulkan arch: ${VULKAN_ARCHITECTURE}")
message (STATUS " + Vulkan root: ${VULKAN_ROOT}")
message (STATUS " + Vulkan debug malloc: ${VULKAN_USE_DEBUG_MALLOC}")
message (STATUS "------------------------------------------------------------")

configure_file ("${EXPORT_H_TEMPLATE}"
                "include/game/export.h")
configure_file ("include/game/config.h.in"
                "include/game/config.h")

include_directories ("${CMAKE_CURRENT_BINARY_DIR}/include")

###############################################################################
# source files and runtime definition
###############################################################################

file (GLOB game_HEADERS "include/game/*.h")
file (GLOB game_SOURCES "src/*.c")
set (game_HEADERS ${game_HEADERS}
    "include/game/config.h.in"
    ${EXPORT_H_TEMPLATE}
)

include_directories ("include")
include_directories ("${VULKAN_ROOT}/${VULKAN_ARCHITECTURE}/include")

add_library (game ${BUILD_TYPE}
    ${game_HEADERS}
    ${game_SOURCES}
)

# The library is being built
add_definitions (-D${PROJECT_NAME}_BUILDING)

if(${PLATFORM} MATCHES "LINUX" OR ${PLATFORM} MATCHES "MACOSX")
    set (MATH_LIB m)
endif ()

target_link_libraries (game ${MATH_LIB})
target_link_libraries (game util)

# generates the correct project files for visual studio, setting the correct
# debug working directory and other stuff
create_vcproj_userfile (game)

add_custom_command (TARGET game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${VULKAN_ROOT}/${VULKAN_ARCHITECTURE}/lib/*" ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_custom_command (TARGET game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${VULKAN_ROOT}/${VULKAN_ARCHITECTURE}/etc/explicit_layer.d/*.json" ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

###############################################################################
# install targets
###############################################################################

# only install if shared library
if (${BUILD_TYPE} STREQUAL "SHARED")
    install (
        TARGETS
            game
        LIBRARY DESTINATION
            "lib"
        RUNTIME DESTINATION
            "bin"
    )
endif ()
