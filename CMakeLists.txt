if(UNIX)
	cmake_minimum_required(VERSION 3.10)
else()
	cmake_minimum_required(VERSION 3.13)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(TianglePPExample VERSION 0.1.0 LANGUAGES CXX)

################################################################################
# Use solution folders feature
################################################################################
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

################################################################################
# Source groups
################################################################################

set(Header_Files
    "trpp_example.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files__trpp
    "dpoint.hpp"
    "tpp_assert.cpp"
    "tpp_assert.hpp"
    "tpp_impl.cpp"
    "tpp_interface.hpp"
    "triangle.h"
    "triangle_impl.hpp"
)
source_group("Source Files\\trpp" FILES ${Source_Files__trpp})

set(Source_Files
    "trpp_example.cpp"
)
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
    ${Header_Files}
    ${Source_Files__trpp}
    ${Source_Files}
)

################################################################################
# Target
################################################################################
if(MSVC)
	# set /SUBSYSTEM:CONSOLE
	add_executable(${PROJECT_NAME} ${ALL_FILES})
else()
	add_executable(${PROJECT_NAME} ${ALL_FILES})
endif()

################################################################################
# Include directories
################################################################################
target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/.;"
)

################################################################################
# Compiler flags
################################################################################
if(MSVC)
   # quiet warnings related to fopen, sscanf, etc.
   target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

################################################################################
# Dependencies
################################################################################
set(ADDITIONAL_LIBRARY_DEPENDENCIES
)
target_link_libraries(${PROJECT_NAME} PUBLIC "${ADDITIONAL_LIBRARY_DEPENDENCIES}")
