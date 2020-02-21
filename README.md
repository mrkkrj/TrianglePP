[comment]: # " ![triangle-PP's logo](triangle-PP-sm.jpg) "
<img src="triangle-PP-sm.jpg" alt="triangle-PP's logo" width="160"/>

Triangle++ (or TrianglePP) is an updated version of Piyush Kumar's ("http://compgeom.com/~piyush) OO/C++ wrapper (https://bitbucket.org/piyush/triangle/overview) for the original 2005 J.P. Shevchuk's *Triangle* package written in C.

It can create Delaunay triangulations and constrained Delaunay triangulations. 

I have ported the original impl. to Visual C++ (VC9, i.e. VisualStudio 2008), extended it for constrainied triangulations and added some bugfixes. 

This code is released under LPGL licence.

## Usage:

For usage patterns see the *trpp_example.cpp* source file. The interface of the wrapper is defined in the *tpp_inteface.hpp* header file.

Additionally, under *testappQt* you'll find a GUI programm to play with the triangulations:

![triangle-PP's GUI test program](triangle-pp-testApp.gif)

CMake support for both the example program and the GUI demonstrator were added too recently.

## Theory:

For for backgroud info on the implementation see "*Triangle: Engineering a 2D Quality Mesh Generator and Delaunay Triangulator*" by JP Shewchuk: http://www.cs.cmu.edu/~quake-papers/triangle.ps.

The original *Triangle* library documentation can be found at: http://www.cs.cmu.edu/~quake/triangle.html.

## Update:

The code is now preliminary ported to x64 Windows. The project file was updated to Visual Studio 2015, the x64 target was added, and the asserts/crashes when running the example program were fixed. It **wasn't thorougly tested though**, and produces some warnings at the moment. 


## TODOs:
 - Test on 64-bit Linux
 - Remove 64-bit warnings
 - Fix: 64-bit version **freezes** for 44 degrees min. angle constraint! Float value range problem?
 - Port the Qt demo app to Emscripten
