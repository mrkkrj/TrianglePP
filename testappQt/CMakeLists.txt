if(UNIX)
	cmake_minimum_required(VERSION 3.10)
else()
	cmake_minimum_required(VERSION 3.13)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(TianglePPTest VERSION 0.1.0 LANGUAGES CXX)

################################################################################
# Use solution folders feature
################################################################################
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

################################################################################
# Use Qt
################################################################################

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

#set(CMAKE_AUTOUIC_SEARCH_PATHS "src/ui")

find_package(Qt5 COMPONENTS Core Widgets Gui REQUIRED)

################################################################################
# Source groups
################################################################################

set(Form_Files
    "./TrianglePPTest.ui"
    "./TrianglePpOptions.ui"
)
source_group("Form Files" FILES ${Form_Files})

set(Resource_Files
    "./TrianglePPTest.qrc"
)
source_group("Resource Files" FILES ${Resource_Files})


################################################################################
# Source groups
################################################################################
set(Header_Files__tpp
)
source_group("Header Files\\tpp" FILES ${Header_Files__tpp})

set(Header_Files
    "DrawingArea.h"
    "TrianglePpOptions.h"
    "TrianglePPTest.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files__tpp
    "../source/tpp_assert.cpp"
    "../source/tpp_impl.cpp"
)
source_group("Source Files\\tpp" FILES ${Source_Files__tpp})

set(Source_Files
    "DrawingArea.cpp"
    "TrianglePpOptions.cpp"
    "TrianglePPTest.cpp"
    "main.cpp"
)
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
    ${Form_Files}
    ${Resource_Files} 
    ${Header_Files}
    ${Header_Files__tpp}
    ${Source_Files}
    ${Source_Files__tpp}
)

################################################################################
# Target
################################################################################
if(MSVC)
	# set /SUBSYSTEM:WINDOWS
	add_executable(${PROJECT_NAME} WIN32 ${ALL_FILES})
else()
	add_executable(${PROJECT_NAME} ${ALL_FILES})
endif()

################################################################################
# Include directories
################################################################################
target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../source"
)
target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../testappQt"
)

################################################################################
# Compiler flags
################################################################################
if(MSVC)
   # quiet warnings related to fopen, sscanf, etc.
   target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
   # add file output as there's no console!
   target_compile_definitions(${PROJECT_NAME} PRIVATE TRIANGLE_DBG_TO_FILE)
endif()

################################################################################
# Dependencies
################################################################################
set(ADDITIONAL_LIBRARY_DEPENDENCIES
    "Qt5::Core;"
    "Qt5::Gui;"
    "Qt5::Widgets"
)
target_link_libraries(${PROJECT_NAME} PUBLIC "${ADDITIONAL_LIBRARY_DEPENDENCIES}")
