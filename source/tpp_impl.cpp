/*! \file tpp_impl.cpp
    \brief The implementation of the 2D Delaunay triangulation class.

    This class is a wrapper on the Triangle package.
 */

#include <iostream>
// changed mrkkrj --
//#include <triangle_impl.hpp>
//#include <tpp_interface.hpp>

// configuaration of the Triangle.h code:
#define NO_TIMER
#define DREDUCED
#define ANSI_DECLARATORS
#define TRILIBRARY
#define TRIFILES_OUTPUT_SUPPORT
#define TRIFILES_READ_SUPPORT
//#define CDT_ONLY // no, we want all algorithms!

#ifndef _WIN64
// the MS x64 compilers do not use FPU (as SSE is the default) thus no extended precision problems!
#  define CPU86
#endif

#ifndef _WIN32
#define LINUX // TEST::: ---> while porting to Linux!!
#undef CPU86
#endif

//#define TRIANGLE_DBG_TO_FILE 1

// mrkkrj::: DEBUG trace 
//    - needed when debugging a GUI app. on Windows without console
#ifdef TRIANGLE_DBG_TO_FILE
#   include <cstdio>

FILE* g_debugFile = nullptr;

// TR string
#   define TRACE(a) { if(g_debugFile) { fprintf(g_debugFile, "%s\n", a); fflush(g_debugFile); } }
   // TR string + integer 
#   define TRACE2i(a,b) { if(g_debugFile) { fprintf(g_debugFile, "%s%d\n", a, b); fflush(g_debugFile); } }
    // TR string + string 
#   define TRACE2s(a,b) { if(g_debugFile) { fprintf(g_debugFile, "%s%s\n", a, b); fflush(g_debugFile); } }
    // TR string + boolean 
#   define TRACE2b(a,b) { if(g_debugFile) { fprintf(g_debugFile, "%s%s\n", a, b ? "true " : "false"); fflush(g_debugFile); } }

#   define INIT_TRACE(a) { g_debugFile = fopen(a, "w"); \
                           if(!g_debugFile) std::cerr << "ERROR: Cannot open trace file: " << a << std::endl; }
#   define END_TRACE() { if(g_debugFile) { fclose(g_debugFile); } }
#else
#   define TRACE(a)
#   define TRACE2i(a,b) 
#   define TRACE2s(a,b) 
#   define TRACE2b(a,b) 
#   define INIT_TRACE(a) 
#   define END_TRACE() 
#endif

#define TRIANGLE_DETAIL_DEBUG 0
// end DEBUG trace (mrkkrj)

#include "triangle_impl.hpp"
#include "tpp_interface.hpp"
#include <sstream>
#include <cassert>
#include <algorithm>
// END changed --


#include <new>

#define REAL double


namespace tpp {

using std::cout;
using std::cerr;

/*!
*/
Delaunay::Delaunay(const std::vector<Point>& points)
   : m_in(nullptr),
     m_triangleWrap(nullptr),
     m_pmesh(nullptr),
     m_pbehavior(nullptr),
     m_triangulated(false),
     m_vorout(nullptr),
     m_minAngle(0.0f),
     m_maxArea(0.0f),
     m_convexHullWithSegments(false)
{
   m_pointList.assign(points.begin(), points.end());
}

/*!
*/
Delaunay::~Delaunay() {
   freeTriangleDataStructs();
}

/*!
*/
void Delaunay::Triangulate(bool quality, DebugOutputLevel traceLvl) {

   std::string options = "nz";  // n: need neighbors, z: index from 0

   setQualityOptions(options, quality);
   setDebugLevelOption(options, traceLvl);

   Triangulate(options);
}

/*!
*/
void Delaunay::TriangulateConf(bool quality, DebugOutputLevel traceLvl) {

   std::string options = "nz";  // n: need neighbors, z: index from 0

   setQualityOptions(options, quality);
   options.append("D"); // conforming Delaunay!
   setDebugLevelOption(options, traceLvl);

   Triangulate(options);
}


/*!
  Triangulate the points stored in m_pointList.
  \note (mrkkrj) copy-pasted from parts of the original Triangle's triangulate() function!
  \author Piyush Kumar (originally), 
          mrkkrj (extracted it, debug output to file, corections, extensions)
*/
void Delaunay::Triangulate(std::string& triswitches) {
    INIT_TRACE("triangle.out.txt");
    TRACE("Triangulate ->");

    if (m_triangulated) {
       freeTriangleDataStructs();
    }

#if TRIANGLE_DETAIL_DEBUG
    size_t posV = triswitches.find("V");
    if(posV != std::string::npos) {
        triswitches.insert(posV, "V"); // detailed trace!
    }
#endif

    m_in = new triangulateio;
    triangulateio* pin = (struct triangulateio *)m_in;
    
    pin->numberofpoints = (int)m_pointList.size();
    pin->numberofpointattributes = (int)0;
    pin->pointlist = static_cast<double *>((void *)(&m_pointList[0]));
    pin->pointattributelist = nullptr;
    pin->pointmarkerlist = nullptr;
    pin->numberofsegments = 0;
    pin->numberofholes = 0;
    pin->numberofregions = 0;
    pin->regionlist = nullptr;

    if (!m_segmentList.empty()) // OPEN:: a separate option to enable segment constraitns???
    {
       pin->numberofsegments = (int)m_segmentList.size() / 2;
       pin->segmentlist = m_segmentList.data(); 
       pin->segmentmarkerlist = nullptr; 

       triswitches.append("p"); // constrained Delaunay (Planar Straight Line Graph)
       triswitches.append("B"); // but no boundary info at the moment!
       if (m_convexHullWithSegments)
       {
          triswitches.append("c"); // -c Encloses the convex hull with segments - (preserve boudnaries in carveholes())  
                                   // If you are refining a mesh, this switch works differently; it generates the set of
                                   // boundary edges of the mesh, including boundaries of holes.
                                   // (https://www.cs.cmu.edu/~quake/triangle.c.html ) could use only during refine instead?
       }
    }

    if (!m_holesList.empty()) // OPEN:: a separate option to enable carving of holes???
    {
       pin->numberofholes = (int)m_holesList.size();
       pin->holelist = static_cast<double*>((void*)(&m_holesList[0]));

       if (m_segmentList.empty())
       {
          triswitches.append("p"); // constrained Delaunay (Planar Straight Line Graph)
          triswitches.append("B"); // but no boundary info at the moment!
          if (m_convexHullWithSegments)
          {
             triswitches.append("c"); // -c Encloses the convex hull with segments - (preserve boundaries in carveholes())
          }
       }
    }

    TRACE2s(" -- switches:", triswitches.c_str());

    m_triangleWrap = new Triwrap;
    Triwrap* pTriangleWrap = (Triwrap *)m_triangleWrap;
    triswitches.push_back('\0');
    char *pTriswitches = &triswitches[0];

    m_pmesh = new Triwrap::__pmesh;
    m_pbehavior = new Triwrap::__pbehavior;

    Triwrap::__pmesh     * tpmesh     = (Triwrap::__pmesh *)     m_pmesh;
    Triwrap::__pbehavior * tpbehavior = (Triwrap::__pbehavior *) m_pbehavior;

    // parse the options:
    pTriangleWrap->parsecommandline(1, &pTriswitches, tpbehavior);

    // initialize data structs
    pTriangleWrap->triangleinit(tpmesh);
    tpmesh->steinerleft = tpbehavior->steiner; // added mrkkrj

    pTriangleWrap->transfernodes(
                tpmesh, tpbehavior, pin->pointlist, 
                pin->pointattributelist,
                pin->pointmarkerlist, pin->numberofpoints,
                pin->numberofpointattributes);

    // triangulate!
    tpmesh->hullsize = pTriangleWrap->delaunay(tpmesh, tpbehavior);

    // OPEN TODO:: mrkkrj
    //    if(concave hull) - compute concave hull with the chi-algorithm,
    //                     - use it as segments in formskeleton()!!
    // end TODO::

    // Ensure that no vertex can be mistaken for a triangular bounding 
    //   box vertex in insertvertex().
    tpmesh->infvertex1 = nullptr;
    tpmesh->infvertex2 = nullptr;
    tpmesh->infvertex3 = nullptr;

    // added mrkkrj: support for the "-q" option
    if (tpbehavior->usesegments && (tpmesh->triangles.items > 0)) {
        tpmesh->checksegments = 1;          /* Segments will be introduced next. */
        if (!tpbehavior->refine) {
          /* Insert PSLG segments and/or convex hull segments. */
          pTriangleWrap->formskeleton(tpmesh, tpbehavior, pin->segmentlist,
                                  pin->segmentmarkerlist, pin->numberofsegments);
        }
    }

    if (tpbehavior->quality && (tpmesh->triangles.items > 0)) {
        pTriangleWrap->enforcequality(tpmesh, tpbehavior);        /* Enforce angle and area constraints. */
    }

    // mrkkrj
    if (tpbehavior->poly && (tpmesh->triangles.items > 0)) 
    {
       tpmesh->holes = pin->numberofholes;
       double* holelist = pin->holelist;

       tpmesh->regions = 0;
       double* regionlist = nullptr; // not yet supported

        if (!tpbehavior->refine) {
          /* Carve out holes and concavities. */
          pTriangleWrap->carveholes(tpmesh, tpbehavior, holelist, tpmesh->holes, regionlist, tpmesh->regions);
        }
    } 

    /* Calculate the number of edges. */
    tpmesh->edges = (3l * tpmesh->triangles.items + tpmesh->hullsize) / 2l;
    pTriangleWrap->numbernodes(tpmesh, tpbehavior);
    TRACE2i("<- Triangulate: triangles= ", tpmesh->triangles.items);

    m_triangulated = true;
    END_TRACE();
}


/*!
  added mrkkrj:
*/
void Delaunay::Tesselate(bool useConformingDelaunay, DebugOutputLevel traceLvl) 
{
   std::string options = "nz";  // n: need neighbors, z: index from 0
   setDebugLevelOption(options, traceLvl);

   // "If the triangulated domain is"
   //"  convex and has no holes, you can use -D switch to force Triangle to"
   //"  construct a conforming Delaunay triangulation instead of a CCDT, so the"
   //"  Voronoi diagram will be valid."

   //options.append("D"); // Voronoi precondition ??? not really!!!
   if (useConformingDelaunay)
   {
      // an option for experimenting!
      options.append("D");
   }
   options.append("v"); // Voronoi

   Triangulate(options);

   // now use the triangulation for a Voronoi diagram
   Triwrap* pTriangleWrap = (Triwrap*)m_triangleWrap;

   Triwrap::__pmesh* tpmesh = (Triwrap::__pmesh*)     m_pmesh;
   Triwrap::__pbehavior* tpbehavior = (Triwrap::__pbehavior*) m_pbehavior;

   // OPEN TODO::: check these preconditions??
   if (tpmesh->holes != 0) {

   }

   m_vorout = new triangulateio;
   triangulateio* pvorout = (struct triangulateio*)m_vorout;

   pvorout->numberofpoints = tpmesh->triangles.items;
   pvorout->numberofpointattributes = tpmesh->nextras;
   pvorout->numberofedges = tpmesh->edges;

   pvorout->pointlist = nullptr;
   pvorout->pointattributelist = nullptr;
   pvorout->pointmarkerlist = nullptr;
   pvorout->numberofsegments = 0;
   pvorout->numberofholes = 0;
   pvorout->numberofregions = 0;
   pvorout->regionlist = nullptr;
   pvorout->edgelist = nullptr;
   pvorout->edgemarkerlist = nullptr;
   pvorout->normlist = nullptr;

   pTriangleWrap->writevoronoi(
         tpmesh, tpbehavior,
         &pvorout->pointlist, &pvorout->pointattributelist,
         &pvorout->pointmarkerlist, &pvorout->edgelist,
         &pvorout->edgemarkerlist, &pvorout->normlist);
}

/*!
  added mrkkrj:
*/
bool Delaunay::checkConstraints(bool& possible) const
{
   //"     If the minimum angle is 28.6"
   //"        degrees or smaller, Triangle is mathematically guaranteed to"
   //"        terminate (assuming infinite precision arithmetic--Triangle may"
   //"        fail to terminate if you run out of precision).  In practice,"
   //"        Triangle often succeeds for minimum angles up to 34 degrees.  For"
   //"        some meshes, however, you might need to reduce the minimum angle to"
   //"        avoid problems associated with insufficient floating-point"
   //"        precision."
   if (m_minAngle <= 28.6)
   {
      return true;
   }
   else
   {
      possible = (m_minAngle <= 34.0);
      return false;
   }   
}

/*!
  added mrkkrj:
*/
bool Delaunay::checkConstraintsOpt(bool relaxed) const
{
   bool possible = false;
   bool ret = checkConstraints(possible);

   if (!ret && relaxed)
   {
      return possible;
   }
   else
   {
      return ret;
   }
}

/*!
  added mrkkrj:
*/
void Delaunay::getMinAngleBoundaries(float& guaranteed, float& possible)
{
   // see above:
   guaranteed = 28.6f;
   possible = 34.0f;
}

/*!
  added mrkkrj:
*/
bool Delaunay::setSegmentConstraint(const std::vector<Point>& segments)
{
   m_segmentList.clear();
   m_segmentList.reserve(segments.size());

   // OPEN TODO::: optimize - unquadrat it...
   for (int i = 0; i < segments.size(); ++i)
   {
      const std::vector<Point>::iterator it = std::find(m_pointList.begin(), m_pointList.end(), segments[i]);
      if (it == m_pointList.end())
      {
         m_segmentList.clear();
         return false;
      }
      else
      {
         m_segmentList.push_back(std::distance(m_pointList.begin(), it));
      }
   }

   // OPEN TODO::: check the intersection constraints ...

   return true;
}


/*!
  added mrkkrj:
*/
bool Delaunay::setSegmentConstraint(const std::vector<int>& segmentPointIndexes)
{
   m_segmentList.clear();
   m_segmentList.reserve(segmentPointIndexes.size());

   for (int i = 0; i < segmentPointIndexes.size(); ++i)
   {      
      const int& pointIdx = segmentPointIndexes[i];
      if (pointIdx < 0 ||
          pointIdx >= m_pointList.size())
      {
         m_segmentList.clear();
         return false;
      }
      else
      {
         m_segmentList.push_back(pointIdx);
      }
   }

   // OPEN TODO::: check for intersections!!!! 

   return true;
}


/*!
  added mrkkrj:
*/
void Delaunay::useConvexHullWithSegments(bool useConvexHull)
{
#if 0
    // --> Yejneshwar commented on 26 Feb 22
    //Hi,
    //    Thank you for this implementation.
    // 
    //    could you please add support for the D tag ?
    // 
    //    i.e.remove concavities
    //
    //    EDIT :
    //    I have fixed the issue, will be submitting a pull request soon.
    //
    //    Support for it existed but internally the "c" tag would still be added which caused the concavities to not be removed.
    //

    TriangulateConf(options); //created a new function just for the -D tag, because the -c tag doesn't remove concavities with a boundary defined through the segment list.
#endif

    m_convexHullWithSegments = useConvexHull; // with/without convex hull (credits Yejneshwar!)
}


/*!
  added mrkkrj:
*/
bool Delaunay::setHolesConstraint(const std::vector<Point>& holes)
{
   m_holesList = holes;

   // OPEN TODO::: check the intersection constraints ...

   // TEST::: make them also points???
   //m_pointList.insert(m_pointList.end(), holes.begin(), holes.end());

   return true;
}


/*!
  added mrkkrj:
*/
bool Delaunay::savePoints(const char* filePath)
{
   if (!m_triangulated) 
   {
     struct triangulateio input;
     triangulateio* pin = (struct triangulateio *)&input;
    
     pin->numberofpoints = (int)m_pointList.size();
     pin->numberofpointattributes = (int)0;
     pin->pointlist = static_cast<double *>((void *)(&m_pointList[0]));
     pin->pointattributelist = nullptr;
     pin->pointmarkerlist = nullptr;
     pin->numberofsegments = 0;
     pin->numberofholes = 0;
     pin->numberofregions = 0;
     pin->regionlist = nullptr;


     if (!m_triangleWrap) 
     {
        m_triangleWrap = new Triwrap;
     }

      m_pmesh = new Triwrap::__pmesh;
      m_pbehavior = new Triwrap::__pbehavior;

      Triwrap* pTriangleWrap = (Triwrap*)m_triangleWrap;

      Triwrap::__pmesh* tpmesh = (Triwrap::__pmesh*)m_pmesh;
      Triwrap::__pbehavior* tpbehavior = (Triwrap::__pbehavior*)m_pbehavior;

      // initialize data structs

      pTriangleWrap->triangleinit(tpmesh);
      tpmesh->steinerleft = tpbehavior->steiner; // added mrkkrj

      pTriangleWrap->transfernodes(
                tpmesh, tpbehavior, pin->pointlist, 
                pin->pointattributelist,
                pin->pointmarkerlist, pin->numberofpoints,
                pin->numberofpointattributes);

      pTriangleWrap->initializetrisubpools(tpmesh, tpbehavior);
   }

   Triwrap* pTriangleWrap = (Triwrap*)m_triangleWrap;

   Triwrap::__pmesh* tpmesh = (Triwrap::__pmesh*)m_pmesh;
   Triwrap::__pbehavior* tpbehavior = (Triwrap::__pbehavior*)m_pbehavior;    
       
   pTriangleWrap->writenodes2file(tpmesh, tpbehavior, const_cast<char*>(filePath), 
                                   0, nullptr); // argc & argv

   return true;
}


/*!
  added mrkkrj:
*/
bool Delaunay::saveSegments(const char* filePath)
{
   // OPEN TODO::: experimental only!!!!!

   if (!m_triangulated) 
   {
    struct triangulateio input;
    triangulateio* pin = (struct triangulateio *)&input;
    
    pin->numberofpoints = (int)m_pointList.size();
    pin->numberofpointattributes = (int)0;
    pin->pointlist = static_cast<double *>((void *)(&m_pointList[0]));
    pin->pointattributelist = nullptr;
    pin->pointmarkerlist = nullptr;
    pin->numberofsegments = 0;
    pin->numberofholes = 0;
    pin->numberofregions = 0;
    pin->regionlist = nullptr;

    if (!m_segmentList.empty()) 
    {
       pin->numberofsegments = (int)m_segmentList.size() / 2;
       pin->segmentlist = m_segmentList.data(); 
       pin->segmentmarkerlist = nullptr; 
    }

   if (!m_triangleWrap) 
   {
      m_triangleWrap = new Triwrap;
   }

    m_pmesh = new Triwrap::__pmesh;
    m_pbehavior = new Triwrap::__pbehavior;

    Triwrap* pTriangleWrap = (Triwrap*)m_triangleWrap;

    Triwrap::__pmesh* tpmesh = (Triwrap::__pmesh*)m_pmesh;
    Triwrap::__pbehavior* tpbehavior = (Triwrap::__pbehavior*)m_pbehavior;  

    // initialize data structs

    pTriangleWrap->triangleinit(tpmesh);
    tpmesh->steinerleft = tpbehavior->steiner; // added mrkkrj

    pTriangleWrap->transfernodes(
                tpmesh, tpbehavior, pin->pointlist, 
                pin->pointattributelist,
                pin->pointmarkerlist, pin->numberofpoints,
                pin->numberofpointattributes);

    if (!m_segmentList.empty())
    {
        // npt working!
        tpbehavior->usesegments = 1;

        pTriangleWrap->formskeleton(tpmesh, tpbehavior, pin->segmentlist,
                                      pin->segmentmarkerlist, pin->numberofsegments);
    }

    pTriangleWrap->initializetrisubpools(tpmesh, tpbehavior);
   }
   // OPEN TODO::: end...


    Triwrap* pTriangleWrap = (Triwrap*)m_triangleWrap;

    Triwrap::__pmesh* tpmesh = (Triwrap::__pmesh*)m_pmesh;
    Triwrap::__pbehavior* tpbehavior = (Triwrap::__pbehavior*)m_pbehavior;

    int numberofholes = 0;
    double* holelist = nullptr;

    if (!m_holesList.empty()) 
    {
        numberofholes = (int)m_holesList.size();
        holelist = static_cast<double*>((void*)(&m_holesList[0]));
    }

    // OPEN TODO:::: regions support???
    int numberofregions = 0;
    double* regionlist = nullptr;

    pTriangleWrap->writepoly2file(tpmesh, tpbehavior, const_cast<char*>(filePath),
                                  holelist, numberofholes, regionlist, numberofregions,
                                  0, nullptr); // argc & argv

   // OPEN TODO:::
   return true;
}

/*!
  added mrkkrj:
*/
bool Delaunay::readPoints(const char* filePath, std::vector<Delaunay::Point>& points)
{
    // OPEN TODO::: experimental only!!!!!
   
    typedef Triwrap::vertex vertex;

    if (!m_triangleWrap)
    {
        m_triangleWrap = new Triwrap;
    }

    m_pmesh = new Triwrap::__pmesh;
    m_pbehavior = new Triwrap::__pbehavior;

    Triwrap::__pmesh* tpmesh = (Triwrap::__pmesh*)m_pmesh;
    Triwrap::__pbehavior* tpbehavior = (Triwrap::__pbehavior*)m_pbehavior;
    Triwrap* pTriangleWrap = (Triwrap*)m_triangleWrap;

    // get points

    pTriangleWrap->triangleinit(tpmesh); 
    pTriangleWrap->initializetrisubpools(tpmesh, tpbehavior);

    tpbehavior->poly = 0; // poly file not provided!
    tpbehavior->usesegments = 0;
    FILE* polyfile = nullptr;

    pTriangleWrap->readnodes(tpmesh, tpbehavior, const_cast<char*>(filePath), nullptr, &polyfile);

    // read points from the mesh data

    m_pointList.clear();
    m_pointList.reserve(tpmesh->invertices);

  int vertexnumber = tpbehavior->firstnumber;
  Triwrap::__pmesh* m = tpmesh; // for Triwrap's macros: vertextype(), setvertexmark()

  pTriangleWrap->traversalinit(&tpmesh->vertices);
  vertex vertexloop = pTriangleWrap->vertextraverse(tpmesh);  

  while (vertexloop != (vertex) NULL) {
    if (!tpbehavior->jettison || (vertextype(vertexloop) != UNDEADVERTEX)) {
      /* X and Y coordinates. */
      m_pointList.push_back({vertexloop[0], vertexloop[1]});

#if 0 // --> not yet supported!
      /* Vertex attributes. */
      for (i = 0; i < tpmesh->nextras; i++) {
        palist[attribindex++] = vertexloop[2 + i];
      }
      if (!tpbehavior->nobound) {
        /* Copy the boundary marker. */
        pmlist[vertexnumber - tpbehavior->firstnumber] = vertexmark(vertexloop);
      }
#endif

      setvertexmark(vertexloop, vertexnumber);
      vertexnumber++;
    }
    vertexloop = pTriangleWrap->vertextraverse(tpmesh);
  }

   // OPEN TODO::
   points = m_pointList; // OPEN TODO::: make optional parameter?????

   return true;
}


/*!
    Write the triangulation to an .off file
     - OFF stands for the Object File Format, a format used by Geometry Center's "Geomview" package. 
*/
void Delaunay::writeoff(std::string& fname){
    if(!m_triangulated) {
        cerr << "FATAL: Write called before triangulation\n";
        //exit(1);
        throw std::runtime_error("FATAL: Write called before triangulation");
    }

    Triwrap::__pmesh     * tpmesh     = (Triwrap::__pmesh *)     m_pmesh;
    Triwrap::__pbehavior * tpbehavior = (Triwrap::__pbehavior *) m_pbehavior;

    Triwrap *pTriangleWrap =  (Triwrap *)m_triangleWrap;
    char *pfname = new char[fname.size()+1];
    strcpy(pfname , fname.c_str());

    pTriangleWrap->writeoff(tpmesh, tpbehavior, pfname, 0, nullptr);
    delete [] pfname;
}

/*!
*/
int Delaunay::nedges() const {
    Triwrap::__pmesh     * tpmesh     = (Triwrap::__pmesh *)     m_pmesh;
    return tpmesh->edges;
}

/*!
*/
int Delaunay::ntriangles() const {
    Triwrap::__pmesh     * tpmesh     = (Triwrap::__pmesh *)     m_pmesh;
    return tpmesh->triangles.items;	
}

/*!
*/
int Delaunay::nvertices() const {
    Triwrap::__pmesh     * tpmesh     = (Triwrap::__pmesh *)     m_pmesh;
    Triwrap::__pbehavior * tpbehavior = (Triwrap::__pbehavior *) m_pbehavior;
    int outvertices;

    if (tpbehavior->jettison) {
        outvertices = tpmesh->vertices.items - tpmesh->undeads;
    } else {
        outvertices = tpmesh->vertices.items;
    }

    return outvertices;
}

/*!
*/
int Delaunay::hull_size() const {
    Triwrap::__pmesh     * tpmesh     = (Triwrap::__pmesh *)     m_pmesh;
    return  tpmesh->hullsize;	
}

/*!
*/
int Delaunay::vertexId(vIterator const &vit) const {
    Triwrap::__pmesh     * tpmesh     = (Triwrap::__pmesh *) vit.MyDelaunay->m_pmesh;
    return ((int *)vit.vloop)[tpmesh->vertexmarkindex];
}

/*!
  added mrkkrj:
*/
int Delaunay::nvedges() const {
   triangulateio* pvorout = (struct triangulateio*)m_vorout;
   if (!pvorout) {
      return 0;
   }
   else {
      return pvorout->numberofedges;
   }
}


/*!
  added mrkkrj:
*/
int Delaunay::nvpoints() const {
   triangulateio* pvorout = (struct triangulateio*)m_vorout;
   if (!pvorout) {
      return 0;
   }
   else {
      return pvorout->numberofpoints;
   }
}

/*!
  added mrkkrj:
*/
int Delaunay::nholes() const {
    Triwrap::__pmesh* tpmesh = (Triwrap::__pmesh*)m_pmesh;
    return tpmesh->holes;
}

/*!
  added mrkkrj:
*/
std::string Delaunay::formatFloatConstraint(float f) const {
    std::ostringstream ss;
    ss << f;
    return ss.str();
}

/*!
  added mrkkrj:
*/
void Delaunay::setDebugLevelOption(std::string& options, DebugOutputLevel traceLvl) {
   switch (traceLvl) {
   case None:
      options.append("Q"); // Q: no trace, no debug
      break;
   case Info:
      options.append("V"); // trace & debug
      break;
   case Vertex:
      options.append("VV"); // more trace & debug
      break;
   case Debug:
      options.append("VVVV"); // much, much more!
      break;
   default:
      assert(false && "unknown trace level");
   }
}

/*!
  added mrkkrj:
*/
void Delaunay::setQualityOptions(std::string& options, bool quality)
{
    if (quality) {
        options.append("q");
        if (m_minAngle > 0) {
            options.append(formatFloatConstraint(m_minAngle));
        }
        if (m_maxArea > 0) {
            options.append("a" + formatFloatConstraint(m_maxArea));
        }
    }
}

/*!
  added mrkkrj:
*/
void Delaunay::freeTriangleDataStructs()
{
   if (m_in == nullptr && m_vorout == nullptr && 
       m_triangleWrap == nullptr && m_pmesh == nullptr &&
       m_pbehavior == nullptr)
   {
      return; // already freed!
   }

   struct triangulateio* pin = (struct triangulateio*) m_in;
   struct triangulateio* pvorout = (struct triangulateio*) m_vorout;

   Triwrap* pTriangleWrap = (Triwrap*)m_triangleWrap;

   Triwrap::__pmesh* tpmesh = (Triwrap::__pmesh*) m_pmesh;
   Triwrap::__pbehavior* tpbehavior = (Triwrap::__pbehavior*) m_pbehavior;

   pTriangleWrap->triangledeinit(tpmesh, tpbehavior);

   delete tpmesh;
   delete tpbehavior;
   delete pin;
   delete pvorout;
   delete pTriangleWrap;

   m_in = nullptr;
   m_vorout = nullptr;
   m_triangleWrap = nullptr;
   m_pmesh = nullptr;
   m_pbehavior = nullptr;

}

///////////////////////////////
//
// Vertex Iterator Impl.
//
///////////////////////////////

/*!
*/
Delaunay::vIterator::vIterator(Delaunay* triangulator) {
     typedef Triwrap::vertex vertex;
     MyDelaunay = triangulator;

     Triwrap::__pmesh     * tpmesh     = (Triwrap::__pmesh *) triangulator->m_pmesh;
     Triwrap::__pbehavior * tpbehavior = (Triwrap::__pbehavior *) triangulator->m_pbehavior;
     Triwrap *pTriangleWrap =  (Triwrap *) triangulator->m_triangleWrap;

     pTriangleWrap->traversalinit(&( tpmesh->vertices ) );
     vloop = pTriangleWrap->vertextraverse(tpmesh);

     while
        (
         tpbehavior->jettison || 
        (
          ((int *)vloop)[tpmesh->vertexmarkindex+1] == UNDEADVERTEX
        )
    ) 
    vloop = (void *) pTriangleWrap->vertextraverse(tpmesh);
}

/*!
*/
Delaunay::vIterator::~vIterator(){
}

/*!
*/
Delaunay::vIterator Delaunay::vend(){
    vIterator vit;
    vit.vloop = ((Triwrap::vertex) nullptr);
    return vit;
}

/*!
*/
Delaunay::vIterator Delaunay::vIterator::operator++() {
     typedef Triwrap::vertex vertex;	

     Triwrap::__pmesh     * tpmesh     = (Triwrap::__pmesh *) MyDelaunay->m_pmesh;
     Triwrap::__pbehavior * tpbehavior = 
                (Triwrap::__pbehavior *) MyDelaunay->m_pbehavior;

     Triwrap *pTriangleWrap =  (Triwrap *) MyDelaunay->m_triangleWrap;

     while
        (
             tpbehavior->jettison ||
            (
              ((int *)vloop)[tpmesh->vertexmarkindex+1] == UNDEADVERTEX
            )
        )
        vloop = (void *) pTriangleWrap->vertextraverse(tpmesh);
        vloop = (void *) pTriangleWrap->vertextraverse(tpmesh);

        vIterator vit;
        vit.vloop = vloop;
        vit.MyDelaunay = MyDelaunay;

        return vit;
}

/*!
*/
Delaunay::Point & Delaunay::vIterator::operator*() const{
    return *((Point *)vloop);
}

/*!
*/
bool operator==(Delaunay::vIterator const &vit1,
                Delaunay::vIterator const &vit2) {
    if (vit1.vloop == vit2.vloop) return true;
    return false;
}

/*!
*/
bool operator!=(Delaunay::vIterator const &vit1,
                Delaunay::vIterator const &vit2) {
    if (vit1.vloop != vit2.vloop) return true;
    return false;
}


///////////////////////////////
//
// Face Iterator Impl.
//
///////////////////////////////

/*!
*/
Delaunay::fIterator::fIterator(Delaunay* triangulator) {
     typedef Triwrap::vertex vertex;
     typedef Triwrap::__otriangle trianglelooptype; // oriented triangle

     MyDelaunay = triangulator;

     Triwrap::__pmesh     * tpmesh     = (Triwrap::__pmesh *) triangulator->m_pmesh;
     Triwrap *pTriangleWrap =  (Triwrap *)triangulator->m_triangleWrap;

     pTriangleWrap->traversalinit(&( tpmesh->triangles ) );
     // floop = new trianglelooptype;
     trianglelooptype *ploop = (trianglelooptype *)(&floop);
     ploop->tri    = pTriangleWrap->triangletraverse(tpmesh);
     ploop->orient = 0;

}

/*!
*/
Delaunay::fIterator::~fIterator(){
}

/*!
*/
Delaunay::fIterator Delaunay::fend(){
    fIterator fit;
    typedef Triwrap::__otriangle trianglelooptype;
    fit.floop.tri = (double ***) nullptr;
    return fit;
}

/*!
*/
void Delaunay::fIterator::operator++() {
     // cout << "++ called\n";
     typedef Triwrap::vertex   vertex;
     typedef Triwrap::triangle triangle;
     typedef Triwrap::__otriangle trianglelooptype;

     Triwrap::__pmesh     * tpmesh     = (Triwrap::__pmesh *) MyDelaunay->m_pmesh;
     
     trianglelooptype *ploop = (trianglelooptype *)(&floop);
     Triwrap *pTriangleWrap =  (Triwrap *) MyDelaunay->m_triangleWrap;

     ploop->tri = pTriangleWrap->triangletraverse(tpmesh);
     // cout << "tri val = " << ploop->tri << endl;
}

/*!
*/
bool operator==(Delaunay::fIterator const &fit1,
                Delaunay::fIterator const &fit2) {

    return (fit1.floop.tri == fit2.floop.tri);

}

/*!
*/
bool operator!=(Delaunay::fIterator const &fit1,
                Delaunay::fIterator const &fit2) {
        return !( operator==(fit1,fit2) );
}

/*!
  added mrkkrj:
*/
bool operator<(Delaunay::fIterator const &fit1,
               Delaunay::fIterator const &fit2) {
        return (fit1.floop.tri < fit2.floop.tri);

}

// 2 helpers for the following methods
//  (added mrkkrj)

/*!
*/
void Delaunay::SetPoint(Point& point, /*Triwrap::vertex*/ double* vertexptr){
    // OPEN TODO: compile test type check - Triwrap::vertex == double* ???
    point[0] = (vertexptr)[0]; // x
    point[1] = (vertexptr)[1]; // y
}

/*!
*/
int Delaunay::GetVertexIndex(fIterator const & fit, /*Triwrap::vertex*/ double* vertexptr){
    // OPEN TODO: compile test type check - Triwrap::vertex == double* ???
     Triwrap::__pbehavior * tpbehavior = (Triwrap::__pbehavior *)
                                                ((fit.MyDelaunay)->m_pbehavior);
     Triwrap::__pmesh  * tpmesh  = (Triwrap::__pmesh *) (fit.MyDelaunay->m_pmesh);

     int ret =
        ( ((int *) (vertexptr))[tpmesh->vertexmarkindex] )
         -
        tpbehavior->firstnumber;

     return ((unsigned)ret < m_pointList.size()) ? ret : -1;
}

/*!
*/
int Delaunay::GetFirstIndexNumber() const {
   Triwrap::__pbehavior* pbehavior = (Triwrap::__pbehavior*)m_pbehavior;
   return pbehavior->firstnumber;
}

/*!
  A triangle abc has origin (org) a, destination (dest) b, and apex (apex)   
  c.  These vertices occur in counterclockwise order about the triangle.
*/
int Delaunay::Org(fIterator const & fit, Point* point){
     typedef Triwrap::vertex   vertex;
     typedef Triwrap::triangle triangle;
     typedef Triwrap::__otriangle trianglelooptype;

     trianglelooptype * ploop   = (trianglelooptype *)(&(fit.floop));

     vertex vertexptr = (vertex) ((ploop->tri)[plus1mod3[ploop->orient] + 3]);
     
     if(point) SetPoint(*point, vertexptr);
     return GetVertexIndex(fit, vertexptr); 
}

/*!
*/
int Delaunay::Dest(fIterator const & fit, Point* point){
     typedef Triwrap::vertex   vertex;
     typedef Triwrap::triangle triangle;
     typedef Triwrap::__otriangle trianglelooptype;

     trianglelooptype * ploop   = (trianglelooptype *)(&(fit.floop));

     vertex vertexptr = (vertex) ((ploop->tri)[minus1mod3[ploop->orient] + 3]);
     
     if(point) SetPoint(*point, vertexptr);
     return GetVertexIndex(fit, vertexptr); 
}

/*!
*/
int Delaunay::Apex(fIterator const & fit, Point* point){
     typedef Triwrap::vertex   vertex;
     typedef Triwrap::triangle triangle;
     typedef Triwrap::__otriangle trianglelooptype;

     trianglelooptype * ploop   = (trianglelooptype *)(&(fit.floop));

     vertex vertexptr = (vertex) ((ploop->tri)[ploop->orient + 3]);

     if(point) SetPoint(*point, vertexptr);
     return GetVertexIndex(fit, vertexptr); 
}

/*!
*/
int Delaunay::Sym(fIterator const & fit, char i){
     typedef Triwrap::vertex      vertex;
     typedef Triwrap::triangle    triangle;
     typedef Triwrap::__otriangle trianglelooptype;
     triangle ptr;                         /* Temporary variable used by sym(). */

     //Triwrap::__pbehavior * tpbehavior = (Triwrap::__pbehavior *)
     //                                           ((fit.MyDelaunay)->pbehavior);

     Triwrap::__pmesh  * tpmesh  = (Triwrap::__pmesh *) (fit.MyDelaunay->m_pmesh);
     trianglelooptype * ploop   = (trianglelooptype *)(&(fit.floop));

     char oval = (char)ploop->orient;
     ploop->orient = i;

     trianglelooptype top;
     sym(*ploop, top);
     ploop->orient = oval;

     if(top.tri != tpmesh->dummytri){
         vertex farvertex;
         apex(top, farvertex);
         return ((int *)farvertex)[tpmesh->vertexmarkindex];
     }
     
    return -1;	 
}

/*!
*/
Delaunay::fIterator Delaunay::Sym(fIterator const & fit){
     fIterator retval;
     retval.MyDelaunay = fit.MyDelaunay;

     typedef Triwrap::vertex      vertex;
     typedef Triwrap::triangle    triangle;
     typedef Triwrap::__otriangle trianglelooptype;
     triangle ptr;                         /* Temporary variable used by sym(). */

     Triwrap::__pmesh  * tpmesh  = (Triwrap::__pmesh *) (fit.MyDelaunay->m_pmesh);
     trianglelooptype * ploop   = (trianglelooptype *)(&(fit.floop));

     
     trianglelooptype top;
     sym(*ploop, top);
     

     if(top.tri != tpmesh->dummytri){
         retval.floop.tri = top.tri;
         retval.floop.orient = top.orient;

         return retval;
     }
     
     retval.floop.tri = nullptr;
     retval.floop.orient = 0;
     return retval;
}

/*!
*/
double Delaunay::area(fIterator const & fit){
    Point torg, tdest, tapex;
    torg  = point_at_vertex_id(Org(fit));
    tdest = point_at_vertex_id(Dest(fit));
    tapex = point_at_vertex_id(Apex(fit));
    double dxod(torg[0] - tdest[0]);
    double dyod(torg[1] - tdest[1]);
    double dxda(tdest[0] - tapex[0]);
    double dyda(tdest[1] - tapex[1]);

    double area = 0.5 * (dxod * dyda - dyod * dxda);
    return area;
}

/*!
*/
Delaunay::fIterator Delaunay::locate(int vertexid){
    fIterator retval;
    retval.MyDelaunay = this;

    typedef Triwrap::vertex      vertex;
    typedef Triwrap::triangle    triangle;
    typedef Triwrap::__otriangle trianglelooptype;

    trianglelooptype horiz;               /* Temporary variable for use in locate(). */
    triangle ptr;                         /* Temporary variable used by sym(). */

    Triwrap::__pmesh     * tpmesh     = (Triwrap::__pmesh *)     m_pmesh;
    Triwrap::__pbehavior * tpbehavior = (Triwrap::__pbehavior *) m_pbehavior;
    Triwrap  *pTriangleWrap           = (Triwrap *)              m_triangleWrap;

    horiz.tri = tpmesh->dummytri;
    horiz.orient = 0;
    symself(horiz);
    double dv[2];
    dv[0] = m_pointList[vertexid][0];
    dv[1] = m_pointList[vertexid][1];

    /* Search for a triangle containing `newvertex'. */
    int intersect = pTriangleWrap->locate(tpmesh, tpbehavior, dv, &horiz);
    Assert(intersect != Triwrap::ONVERTEX, "Something went wrong in point location\n"); // added mrkkrj
    
    if(intersect != Triwrap::ONVERTEX) { // Not on vertex!
        cout << "Something went wrong in point location\n";
        exit(1);
    }
    
    retval.floop.tri = horiz.tri;
    retval.floop.orient = horiz.orient;

    return retval;
}

/*!
  lnext() finds the next edge (counterclockwise) of a triangle.
*/
Delaunay::fIterator Delaunay::Lnext(fIterator const & fit){
    typedef Triwrap::__otriangle trianglelooptype;

    fIterator retval;
    retval.MyDelaunay = this;

    lnext(   (*(trianglelooptype *)(&(fit.floop))), (*(trianglelooptype *)(&(retval.floop))));
    return retval;
}

/*!
  lprev:  Find the previous edge (clockwise) of a triangle.  
  lprev(abc) -> cab   
*/
Delaunay::fIterator Delaunay::Lprev(fIterator const & fit){
    typedef Triwrap::__otriangle trianglelooptype;

    fIterator retval;
    retval.MyDelaunay = this;

    lprev(   (*(trianglelooptype *)(&(fit.floop))), (*(trianglelooptype *)(&(retval.floop))));
    return retval;
}

/*!
  onext:  Find the next edge counterclockwise with the same origin.
  onext(abc) -> ac*
*/
Delaunay::fIterator Delaunay::Onext(fIterator const & fit){
    typedef Triwrap::__otriangle trianglelooptype;
    typedef Triwrap::triangle    triangle;

    triangle ptr;
    fIterator retval;
    retval.MyDelaunay = this;

    //cout << "Onext called:\n " 
    //	 << Org(fit) << "\t" << Dest(fit) << "\t" << Apex(fit) << "\n";

    onext(   (*(trianglelooptype *)(&(fit.floop))), (*(trianglelooptype *)(&(retval.floop))));

    // retval could be dummy!
    return retval;
}

/*!
  oprev:  Find the next edge clockwise with the same origin. 
  oprev(abc) -> a*b  
*/
Delaunay::fIterator Delaunay::Oprev(fIterator const & fit){
    typedef Triwrap::__otriangle trianglelooptype;
    typedef Triwrap::triangle    triangle;

    triangle ptr;
    fIterator retval;
    retval.MyDelaunay = this;

    oprev(   (*(trianglelooptype *)(&(fit.floop))), (*(trianglelooptype *)(&(retval.floop))));
    return retval;
}

/*!
*/
bool Delaunay::isdummy(fIterator const & fit){
    Triwrap::__pmesh     * tpmesh     = (Triwrap::__pmesh *)     m_pmesh;
    typedef Triwrap::__otriangle trianglelooptype;

    return ( ((trianglelooptype *)(&(fit.floop)))->tri == tpmesh->dummytri );
}

/*!
*/
void Delaunay::trianglesAroundVertex(int vertexid, std::vector<int>& ivv){
    fIterator fit = locate(vertexid);
    ivv.clear();

    int start = Dest(fit);
    int linkn = Apex(fit);

    ivv.push_back(vertexid);
    ivv.push_back(start);
    ivv.push_back(linkn);

    fIterator nfit = fit;
    fIterator pnfit = fit; // follows nfit by one triangle

    while( linkn != start ){
        
        nfit = Onext(nfit);
        if (isdummy(nfit)){
            // Do another algorithm
            ivv.clear();

            // use oprev now...
            fit = pnfit;
            nfit = fit;

            start = Apex(fit);
            linkn = Dest(fit);
            
            ivv.push_back(vertexid);
            ivv.push_back(linkn);
            ivv.push_back(start);
            
            while( linkn != start ){
                nfit = Oprev(nfit);
                if(isdummy(nfit))
                    return;
                int a = Org(nfit);
                int b = Dest(nfit);
                int c = Apex(nfit);
                ivv.push_back(a);
                ivv.push_back(b);
                ivv.push_back(c);
                linkn = Dest(nfit);
            }

            return;
        }
        
        pnfit = nfit;

        int a = Org(nfit);
        int b = Dest(nfit);
        int c = Apex(nfit);

        //cout << "Triangle: " << a << "\t"  << b << "\t"  << c << "\n";

        ivv.push_back(a);
        ivv.push_back(b);
        ivv.push_back(c);

        linkn = Apex(nfit);
    }

}


///////////////////////////////
//
// Voronoi Point Iterator Impl.
//  (added mrkkrj)
//
///////////////////////////////

/*!
*/
Delaunay::vvIterator::vvIterator()
   : m_delaunay(nullptr), vvloop(nullptr), vvindex(0), vvcount(0) {
}

/*!
*/
Delaunay::vvIterator::vvIterator(Delaunay* triangulator) {
   m_delaunay = triangulator;
   triangulateio* pvorout = (struct triangulateio*)triangulator->m_vorout;

   // TEST::: I hope so!
   assert(triangulator->GetFirstIndexNumber() == 0);

   vvloop = pvorout->pointlist;
   vvindex = 0;
   vvcount = pvorout->numberofpoints;
}

/*!
*/
Delaunay::vvIterator Delaunay::vvend() {
   vvIterator vvit;
   vvit.vvloop = nullptr;
   vvit.vvindex = 0;
   vvit.vvcount = 0;
   vvit.m_delaunay = nullptr;
   
   return vvit;
}

/*!
*/
Delaunay::vvIterator Delaunay::vvIterator::operator++() {
   vvIterator vit;
   vit.vvloop = vvloop;
   vit.vvindex = vvindex;
   vit.m_delaunay = m_delaunay;

   advance(1);

   return vit;
}

/*!
*/
Delaunay::Point& Delaunay::vvIterator::operator*() const {
   Point::NT* pointlist = (Point::NT*)vvloop;

   // UB! -> but also in original code...OPEN TODO::: !!!
   return *((Point*)(pointlist + vvindex));
}

/*!
*/
void Delaunay::vvIterator::advance(int steps) {
   int stepSize = 2;
   assert(Point().dim() == stepSize);

   if (vvindex/stepSize + steps < vvcount) {
      vvindex += steps * stepSize;
   }
   else {
      // at end
      vvindex = 0;
      vvloop = nullptr;
   }

   assert(vvindex / stepSize < vvcount);
}

/*!
*/
bool operator==(Delaunay::vvIterator const& lhs,
                Delaunay::vvIterator const& rhs) {
   if (lhs.vvloop == rhs.vvloop && 
       lhs.vvindex == rhs.vvindex) return true;
   return false;
}

/*!
*/
bool operator!=(Delaunay::vvIterator const& lhs,
                Delaunay::vvIterator const& rhs) {
   return !(lhs == rhs);
}


///////////////////////////////
//
// Voronoi Edge Iterator Impl.
//  (added mrkkrj)
//
///////////////////////////////

/*!
*/
Delaunay::veIterator::veIterator()
   : m_delaunay(nullptr), veloop(nullptr), veindex(0), vecount(0) {
}

/*!
*/
Delaunay::veIterator::veIterator(Delaunay* triangulator) {
   m_delaunay = triangulator;
   triangulateio* pvorout = (struct triangulateio*)triangulator->m_vorout;

   // TEST::: I hope so!
   assert(triangulator->GetFirstIndexNumber() == 0);

   veloop = pvorout->edgelist; 
   veindex = 0; 
   vecount = pvorout->numberofedges;
}

/*!
*/
Delaunay::veIterator Delaunay::veend() {
   veIterator veit;
   veit.veloop = nullptr;
   veit.veindex = 0;
   veit.vecount = 0;
   veit.m_delaunay = nullptr;
   
   return veit;
}

/*!
*/
Delaunay::veIterator Delaunay::veIterator::operator++() {
   veIterator veit;
   veit.veloop = veloop;
   veit.veindex = veindex;
   veit.m_delaunay = m_delaunay;

   // an edge is represented as 2 integer indexes!
   if (veindex / 2 + 1 < vecount ) {
       veindex += 2;
   }
   else {
      veindex = 0;
      veloop = nullptr;
   }

   assert(veindex / 2 < vecount);
   return veit;
}

/*!
*/
int Delaunay::veIterator::startPointId() const {
   if (!veloop) {
      assert(false);
      return -1;
   }

   auto edgelist = (int*)veloop;
   return edgelist[veindex];
}

/*!
*/
int Delaunay::veIterator::endPointId(Point& normvec) const {
   if (!veloop) {
      assert(false);
      return -1;
   }

   assert(veindex / 2 < vecount);
   auto edgelist = (int*)veloop;
   int idx = edgelist[veindex + 1];
   
   if (idx == -1) {
      triangulateio* pvorout = (struct triangulateio*)m_delaunay->m_vorout;
      auto normlist = pvorout->normlist;
            
      // normlist has same no. of elements as edgelist!
      normvec[0] = normlist[veindex];
      normvec[1] = normlist[veindex + 1];

      assert(!(normvec[0] == 0.0 && normvec[1] == 0.0));
   }
   else {
      normvec[0] = 0.0;
      normvec[1] = 0.0;
   }

   return idx;
}

/*!
*/
bool operator==(Delaunay::veIterator const& lhs,
                Delaunay::veIterator const& rhs) {
   if (lhs.veloop == rhs.veloop &&
       lhs.veindex == rhs.veindex) return true;
   return false;
}

/*!
*/
bool operator!=(Delaunay::veIterator const& lhs,
                Delaunay::veIterator const& rhs) {
   return !(lhs == rhs);
}

/*!
*/
const Delaunay::Point& Delaunay::Org(veIterator const& eit)
{
   auto pointId = eit.startPointId();

   vvIterator vit(this);
   vit.advance(pointId);
   return *vit;
}

/*!
*/
Delaunay::Point Delaunay::Dest(veIterator const& eit, bool& finiteEdge)
{
   // OPEN TODO::: optimization --- use const& as return value!

   Point normvec;

   auto pointId = eit.endPointId(normvec);
   finiteEdge = pointId != -1;

   if (pointId == -1) {
      assert(normvec.sqr_length() != 0.0);
      return normvec;
   } else {
      assert(normvec.sqr_length() == 0.0);
      
      vvIterator vit(this);
      vit.advance(pointId);
      return *vit;
   }
}

} // namespace tpp ends.
