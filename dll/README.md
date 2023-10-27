# DLL Build

*TrianglePP* can be also used as a DLL/shared library. 

In general, you will have to use following defines

 - TRPP_BUILD_SHARED 
 - TRPP_TRIANGLE_LIB

 to build *TriagnglePP* as a shared library and 

  - TRPP_BUILD_SHARED

  to use it. Example CMake file for DLL builds is provided here. Example project using *TriagnglePP* as a shared library is provided in the *dllUsageTest* subdirectory.


 - **WARNING:** DLL build was only tested on Windows as for now!!!!

