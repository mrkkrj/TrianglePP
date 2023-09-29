![triangle-PP's logo](../triangle-PP-sm.jpg) 
<!-- img src="../triangle-PP-sm.jpg" alt="triangle-PP's logo" width="160"/ -->
## Qt-based GUI Demonstrator for TrianglePP

A small demonstrator/tester for the library.

![triangle-PP info screen](./triangle-PP-info-screen.jpg) 

**Note:** On Windows it is compiled with *TRIANGLE_DBG_TO_FILE* define, so the debug traces will be written to the *./triangle.out.txt* file.

## TODOs:

 - BUG - reading files with holes: hole positions off!!! (tppDataFiles/hex-overlap.poly)

 - remove warnings
 - correct rescaling so that points on the border of frame will be entirely visible (???)
 - rescale the inputs only on explicit request (and back...)
   - OR... add zooming (???)
 
 - add JPG export/screenshot
 - add convex hull demonstration (???)

 - port to Qt 6
 - port demo app to Emscripten
