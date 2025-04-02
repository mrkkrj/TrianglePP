project(TrianglePP)

file(GLOB TPP_SOURCES "source/*.cpp")

# Create a static library from TrianglePP source files
add_library(TrianglePP STATIC ${TPP_SOURCES})

target_include_directories(TrianglePP PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/source)
