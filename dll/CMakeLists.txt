cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(TrianglePP VERSION 0.1.0 LANGUAGES CXX)

################################################################################
# Use solution folders feature
################################################################################
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

################################################################################
# Source groups
################################################################################

set(Header_Files
)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files__trpp
    "../source/dpoint.hpp"
    "../source/tpp_assert.cpp"
    "../source/tpp_assert.hpp"
    "../source/tpp_trace.hpp"
    "../source/tpp_impl.cpp"
    "../source/tpp_interface.hpp"
    "../source/triangle_impl.hpp"
)
source_group("Source Files\\trpp" FILES ${Source_Files__trpp})

set(Source_Files
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
	add_library(${PROJECT_NAME} SHARED ${ALL_FILES})
else()
	add_library(${PROJECT_NAME} SHARED ${ALL_FILES})
endif()


set_target_properties(${PROJECT_NAME} PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib"
)

################################################################################
# Include directories
################################################################################
target_include_directories(${PROJECT_NAME} PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/../source;"
)

################################################################################
# Compiler flags
################################################################################
if(MSVC)
   # quiet warnings related to fopen, sscanf, etc.
   target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS TRPP_BUILD_SHARED TRPP_TRIANGLE_LIB)
else()
   target_compile_definitions(${PROJECT_NAME} PRIVATE TRPP_BUILD_SHARED TRPP_TRIANGLE_LIB)
endif()

################################################################################
# Dependencies
################################################################################
set(ADDITIONAL_LIBRARY_DEPENDENCIES
)
target_link_libraries(${PROJECT_NAME} PUBLIC "${ADDITIONAL_LIBRARY_DEPENDENCIES}")
