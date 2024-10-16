![triangle-PP's logo](../triangle-PP-sm.jpg) 
<!-- img src="../triangle-PP-sm.jpg" alt="triangle-PP's logo" width="160"/ -->
## Qt-based GUI Demonstrator for TrianglePP

A small demonstrator/tester for the library.

![triangle-PP info screen](./triangle-PP-info-screen.jpg) 

Recently this Demo App was ported to Qt6. You will find the legacy Qt5 version on a separate branch, which isn't maintained at the moment, 
i.e. there will be no backports of fixes or new features if not explicitely requested by the users!

**Note:** On Windows Demo App is compiled with *TRIANGLE_DBG_TO_FILE* define, so the debug traces will be written to the *./triangle.out.txt* file.

## TODOs:

 - add visual GUI support for adding region markers and region constraints
 - add GUI options for line, point and segment colours

 - remove warnings
 - correct rescaling so that points on the border of frame will be entirely visible (???)
 - rescale the inputs only on explicit request (and back...) 
   - needed ??? --> we already added zooming
 
 - add JPG export/screenshot
 - add convex hull demonstration (???)

 - port demo app to Emscripten
