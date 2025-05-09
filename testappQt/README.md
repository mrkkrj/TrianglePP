![triangle-PP's logo](../triangle-PP-sm.jpg) 
<!-- img src="../triangle-PP-sm.jpg" alt="triangle-PP's logo" width="160"/ -->
## Qt-based GUI Demonstrator for TrianglePP
<br>

A small demonstrator/tester for the library.
<br>


![triangle-PP info screen](./triangle-PP-info-screen.jpg) 

The Demo App has same options that can be set, available in the *Options* menu (Ctrl+O):

![demmo app options](./demo-app-options.jpg)

 - For more screenshots of what this demonstrator can do, please consult the main *README* file of the project.

**Note:** On Windows the Demo App will be compiled with *TRIANGLE_DBG_TO_FILE* define, so that its debug traces will be written to the *./triangle.out.txt* file.

## Versions
Recently this Demo App was ported to Qt6. You will find the legacy Qt5 version on a separate branch, which isn't maintained at the moment, 
i.e. there will be no backports of fixes or new features if not explicitely requested by the users!

## TODOs:

 - add visual GUI support for adding region markers and region constraints
 - save current settings and re-read them on restart


 - remove warnings
 - correct rescaling so that points on the border of frame will be entirely visible (needed ???)
 - rescale the inputs only on explicit request (and back...) 
   - needed ??? --> we already added zooming

 
 - add JPG export/screenshot
 - add convex hull demonstration (worth it ???)


 - port demo app to Emscripten
 - port demo app to Python
