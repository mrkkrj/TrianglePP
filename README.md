
![triangle-PP's logo](triangle-PP-sm.jpg)
Triangle++ 
==========

This is an updated version of Piyush Kumar's ("http://compgeom.com/~piyush) OO/C++ wrapper (https://bitbucket.org/piyush/triangle/overview) for the original J.P Shevchuk's Triangle package written in C.

It can create Delaunay triangulations and constrained Delaunay triangulations.

I have ported the original impl. to Visual C++ (VC9, i.e. VisStudio 2008), extended it for constrainied triangulations and added some bugfixes. For usage patterns see the *trpp_example.cpp* source file. The interface is defined in *tpp_inteface.hpp*.

Additionally, under *"testappQt"* you'll find a GUI programm to play with the triangulations:

![triangle-PP's GUI test program](triangle-pp-testApp.gif)


For for backgroud info on the implementation see *"Triangle: Engineering a 2D Quality Mesh Generator and Delaunay Triangulator"* by JP Shewchuk: http://www.cs.cmu.edu/~quake-papers/triangle.ps

This code is released under LPGL licence.

## Update:

The code is now preliminary ported to x64 Windows. The project file was updated to Visual Studio 2015, the x64 target was added, and the asserts/crashes when running the example program were fixed. It **wasn't thorougly tested though**, and produces some warnings at the moment. 
