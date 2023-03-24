 /** 
    @file  tpp_interface.hpp    

    @brief Declaration of the main Delaunay class and Iterators of the Triangle++ wrapper
    @copyright  Copyright 20218, Marek Krajewski, released under the terms of LGPL v3

    @author  Marek Krajewski (mrkkrj), www.ib-krajewski.de
    @author  Piyush Kumar (piyush), http://compgeom.com/~piyush
    @author  Jonathan Richard Shewchuk (TriLib!!!), https://people.eecs.berkeley.edu/~jrs/        

    @changes
       11/03/06: piyush - Fixed the compilation system.
       10/25/06: piyush - Wrapped in tpp namespace for usage with other libraries with similar names.
                          Added some more documentation/small changes. Used doxygen 1.5.0 and dot. Tested 
                          compilation with icc 9.0/9.1, gcc-4.1/3.4.6. 
       10/21/06: piyush - Replaced vertexsort with C++ sort.
       08/24/11: mrkkrj - Ported to Visual Studio, added comp. operators, reformatted and added some comments 
       10/15/11: mrkkrj - added support for the "quality triangulation" option, added some debug support
       11/07/11: mrkkrj - bugfix in Triangle's divandconqdelauney() 
       17/09/18: mrkkrj – ported to 64-bit (preliminary, not thorougly tested!) 
       22/01/20: mrkkrj – added support for custom constraints (angle and area) 
       17/04/20: mrkkrj – added support for Voronoi tesselation 
       05/08/22: mrkkrj – added more tests for constrained PSLG triangulations, included (reworked) Yejneshwar's 
                          fix for removal of concavities 
       17/12/22: mrkkrj – Ported to Linux, reworked Yejneshwar's fix again 
       30/12/22: mrkkrj – added first file read-write support 
       03/02/23: mrkkrj – added first support for input sanitization 
       15/03/23: mrkkrj – added support for iteration over the resulting mesh, some refactorings 
 */

#ifndef TRPP_INTERFACE
#define TRPP_INTERFACE

#include "dpoint.hpp"

#include <vector>
#include <string>
#include <unordered_map>

// backwards compatibility:
#define TRPP_OLD_NAMES_SUPPORTED 1

class Triwrap;
struct triangulateio;


namespace tpp 
{
   class vIterator;
   class fIterator;
   class vvIterator;
   class veIterator;
   class TriangulationMesh;
   struct FacesList;

   typedef vIterator VertexIterator;
   typedef fIterator FaceIterator;
   typedef vvIterator VoronoiVertexIterator;
   typedef veIterator VoronoiEdgeIterator;

   enum DebugOutputLevel
   {
      None,
      Info,    // most useful; it gives information on algorithmic progress and much more detailed statistics
      Vertex,  // gives vertex-by-vertex details, and prints so much that Triangle runs much more slowly
      Debug    // gives information only a debugger could love
   };


   /**
      @brief: The main Delaunay class that wraps original Triangle (aka TriLib) code by J.R. Shewchuk

      Use this class to produce Delaunay triangulations (and more...):

        Delaunay d(inputPoints);
        d.Triangulate();
        for(const auto& f: faces()) { ... } // iterates over triangles

      Original TriLib code: http://www.cs.cmu.edu/~quake/triangle.html
      Paper about TriLib impl.: http://www.cs.cmu.edu/~quake-papers/triangle.ps
      
      @note: Currently the dpoint class by Piyush Kumar is used: a d-dimensional reviver::dpoint class 
             with d=2. If you want to use your own point class, you might have to work hard :-(...
    */
   class Delaunay 
   {
   public:
      typedef reviver::dpoint<double, 2> Point; // TODO:: decouple from this dependency!

      /**
         @brief: constructor

         @param points: vector of 2 dimensional points to be used as input
         @param enableMeshIndexing: enables incremental numbering of resulting vertices while iterating over 
                resulting faces/triangles (@see fIterator/FaceIterator)
       */
      Delaunay(const std::vector<Point>& points = std::vector<Point>(), bool enableMeshIndexing = false);

      /**
         @brief: destructor
       */
      ~Delaunay();

      //---------------------------------
      //  main API 
      //---------------------------------

      /**
         @brief: Delaunay triangulate the input points

         This function triangulates points given as input to the constructor of this class. A quality 
         triangualtion can be also created here.

         If segment constraints are set, this method creates a constrained Delaunay triangulation where
         each PSLG segment is present as a single edge in the triangulation. Note that some of the resulting
         triangles might *not be Delaunay*! In quality triangulation *additional* vertices called Steiner 
         points may be created.

         @param quality: enforce minimal angle (default: 20°) and minimal area (default: none)
         @param traceLvl: enable traces
       */
      void Triangulate(bool quality = false, DebugOutputLevel traceLvl = None);

      /**
        @brief: Convenience method
       */
      void Triangulate(DebugOutputLevel traceLvl) { Triangulate(false, traceLvl); }

       /**
          @brief: Conforming Delaunay triangulate the input points 

          This function triangulates points given as input to the constructor of this class, using the segments 
          set with setSegmentConstraint() as constraints. Here a conforming triangualtion will be created.

          A conforming Delaunay triangulation is a *true Delaunay* triangulation in which each constraining 
          segment may have been *subdivided* into several edges by the insertion of *additional* vertices, called 
          Steiner points (@see: http://www.cs.cmu.edu/~quake/triangle.defs.html)

          @param quality: enforce minimal angle (default: 20°) and minimal area (default: none)
          @param traceLvl: enable traces
        */
      void TriangulateConf(bool quality = false, DebugOutputLevel traceLvl = None);

      /**
        @brief: Convenience method
       */
      void TriangulateConf(DebugOutputLevel traceLvl) { TriangulateConf(false, traceLvl); }

      /**
          @brief: Voronoi tesselate the input points

          This function creates a Voronoi diagram for points given as input to the constructor of this 
          class. Note that a Voronoi diagram can be only created if the underlying triangulation is convex
          and doesn't have holes!

          @param useConformingDelaunay: use conforming Delaunay triangulation as base for the Voronoi diagram
          @param traceLvl: enable traces
        */
      void Tesselate(bool useConformingDelaunay = false, DebugOutputLevel traceLvl = None);
    
      /**
        @brief: Enable incremental numbering of vertices in the triangulation while iterating over faces

        @note: must be set before Triangulate() was called to take effect
       */
      void enableMeshIndexGeneration();

      //---------------------------------
      //  constraints API 
      //---------------------------------

      /**
        @brief: Set quality constraints for triangulation

        @param angle: min. resulting angle, if angle <= 0, the constraint will be removed
        @param area:  max. triangle area, if area <= 0, the constraint will be removed
       */
      void setQualityConstraints(float angle, float area);

      /**
        @brief: Convenience method
       */
      void setMinAngle(float angle) { m_minAngle = angle; }

      /**
        @brief: Convenience method
       */
      void setMaxArea(float area) { m_maxArea = area; }

      /**
        @brief: Set the segment constraints for triangulation

        @param segments: vector of 2 dimensional points where each consecutive pair of points describes
                         a single segment. Both endpoints of every segment are vertices of the input vector, 
                         and a segment may intersect other segments and vertices only at its endpoints!
        @return: true if the input is valid, false otherwise 
       */
      bool setSegmentConstraint(const std::vector<Point>& segments);

      /**
        @brief: Same as above, but using indexes of the input points
       */
     bool setSegmentConstraint(const std::vector<int>& segmentPointIndexes, DebugOutputLevel traceLvl = None);

     /**
       @brief: Use convex hull with constraining segments

       @param useConvexHull: if true - generate convex hull using all specified points, the constraining 
                             segments are guaranteed to be included in the triangulation
      */
     void useConvexHullWithSegments(bool useConvexHull);

     /**
       @brief: Set holes to constrain the triangulation

       @param holes: vector of 2 dimensional points where each points marks a hole, i.e. it infects all 
                     triangles around in until it sees a segment
       @return: true if the input is valid, false otherwise
      */
     bool setHolesConstraint(const std::vector<Point>& holes);

     /**
        @brief:  Set a user test function for the triangulation
                 OPEN TODO::: NYI!!!
      */
     void setUserConstraint(bool (*f)()) { /* NYI !!!!! */ }

      /**
        @brief: Are the quality constraints acceptable?

        @param possible: set to true, if is highly *probable* for triangualtion to succeed
        @return: true if triangualtion is *guaranteed* to succeed
       */
      bool checkConstraints(bool& possible) const;

      /**
        @brief: Are the quality constraints acceptable?

        @param relaxed: report highly probable as correct too, as error otherwise
        @return: true if triangualtion is guaranteed or higly probable to succeed
       */
      bool checkConstraintsOpt(bool relaxed) const;

      /**
        @brief: Get the acceptable ranges for quality constraints

        @param guaranteed: up to this value triangualtion is guaranteed to succeed
        @param possible: up to this value it is highly probable for triangualtion to succeed
       */
      static void getMinAngleBoundaries(float& guaranteed, float& possible);

      //---------------------------------
      //  results API 
      //---------------------------------

      /**
        @brief: Is the triangulation completed?
       */
      bool hasTriangulation() const;

      /**
        @brief: Triangulation results, counts of entities:
       */
      int edgeCount() const;
      int triangleCount() const;
      int verticeCount() const;
      int hullSize() const;
      int holeCount() const;

#ifdef TRPP_OLD_NAMES_SUPPORTED
      //!  -> same, for backward comp. only!!! (Will be removed...)
      int nedges() const;
      int ntriangles() const;
      int nvertices() const;
      int hull_size() const;
      int nholes() const;
#endif

      /**
        @brief: Min-max point coordinates values in the resulting triangulation
       */
      void getMinMaxPoints(double& minX, double& minY, double& maxX, double& maxY) const;

      /**
        @brief: Iterate over resulting faces (i.e. triangles) and vertices
       */
      FaceIterator fbegin();
      FaceIterator fend();
      FacesList faces();
      VertexIterator vbegin();
      VertexIterator vend();

      /**
        @brief: Tesselation results, counts of entities:
       */
      int voronoiPointCount() const;
      int voronoiEdgeCount() const;

#ifdef TRPP_OLD_NAMES_SUPPORTED
      //!  -> same, for backward comp. only!!! (Will be removed...)
      int nvpoints() const;
      int nvedges() const;
#endif

      /**
        @brief: Iterate over Voronoi vertices and edges
       */
      VoronoiVertexIterator vvbegin();
      VoronoiVertexIterator vvend();
      VoronoiEdgeIterator vebegin();
      VoronoiEdgeIterator veend();
      
      //---------------------------------
      //  file I/O API 
      //---------------------------------

      /**
        @brief: Write the current vertices to a text file in TriLib's .node file format.

        @param filePath: directory and the name of file to be written
        @return: true if file written, false otherwise
       */
      bool savePoints(const std::string& filePath);

      /**
        @brief: Write the current vertices and segments to a text file in TriLib's .poly file format.

        @param filePath: directory and the name of file to be written
        @return: true if file written, false otherwise
       */
      bool saveSegments(const std::string& filePath);

      /**
        @brief: Write the triangulation to an .off file
        @note: OFF stands for the "Object File Format", a format used by Geometry Center's "Geomview" package.
       */
      void writeoff(std::string& fname);

      /**
        @brief: Read vertices from a text file in TriLib's .node file format.

        @param filePath: directory and the name of file to be read
        @param points: vertices read from the file
        @return: true if file read, false otherwise
       */
      bool readPoints(const std::string& filePath, std::vector<Point>& points);

      /**
        @brief: @brief: Read vertices from a text file in TriLib's .poly file format.

        @param filePath: directory and the name of file to be read
        @param points: vertices read from the file
        @param segmentEndpoints:  ------ OPEN TODO::: comment!!!!!
        @param holeMarkers: coordinates of hole marker points
        @return: true if file read, false otherwise
       */
      bool readSegments(const std::string& filePath, std::vector<Point>& points, std::vector<int>& segmentEndpoints,
                        std::vector<Delaunay::Point>& holeMarkers);

      //---------------------------------
      //  Triangulation mesh access API 
      //---------------------------------

      /**
         @brief:  Point locate a vertex V

         @param vertexId: the vertex
         @return: a face iterator whose origin is V
       */
      FaceIterator locate(int vertexId); // OPEN:: doesn't seem to be working!

      /**
         @brief: Given a vertex index, return the actual Point from the input data
       */
      const Point& pointAtVertexId(int vertexId) const;

      /**
         @brief: Calculate area of a triangle pointed to by an iterator
       */
      double area(FaceIterator const& fit); // OPEN TODO:: move to FaceIter??? TriangulationMesh???

      /**
        @brief: Class for operations on oriented triangles (faces)
       */
      TriangulationMesh mesh();


#ifdef TRPP_OLD_NAMES_SUPPORTED
      //  -> for backward comp. only!!! (Will be removed, use methods of the iterator!)          
      int Org(FaceIterator const& fit, Point* point = 0) const;
      int Dest(FaceIterator const& fit, Point* point = 0) const;
      int Apex(FaceIterator const& fit, Point* point = 0) const;

      const Point& Org(VoronoiEdgeIterator const& eit);
      Point Dest(VoronoiEdgeIterator const& eit, bool& finiteEdge);
#endif

      /**
         @brief: helper, use it to sort the Points first on their X then on Y coord.
                 OPEN:: compiler cannot instantiate less<> with operator<() for Point class?!
       */
      struct OrderPoints
      {
         bool operator() (const Point& lhs, const Point& rhs) const;
      };

   private:
      void triangulateTriLib(std::string& triswitches);
      void setQualityOptions(std::string& options, bool quality);
      void setDebugLevelOption(std::string& options, DebugOutputLevel traceLvl);
      void sanitizeInputData(std::unordered_map<int, int> duplicatePointsMap, DebugOutputLevel traceLvl = None);
      void freeTriangleDataStructs();
      void initTriangleDataForPoints();
      void initTriangleInputData(triangulateio* pin, const std::vector<Point>& points);
      void readPointsFromMesh(std::vector<Point>& points) const;
      void readSegmentsFromMesh(std::vector<int>& segments) const;
      void static SetPoint(Point& point, /*Triwrap::vertex*/ double* vertexptr);

      bool readSegmentsFromFile(char* polyfileName, FILE* polyfile);
      std::string formatFloatConstraint(float f) const;
      std::unordered_map<int, int> checkForDuplicatePoints() const;   
      int GetFirstIndexNumber() const;           

      friend class vIterator;
      friend class fIterator;
      friend class vvIterator;
      friend class veIterator;
      friend class TriangulationMesh;
      
   private:
      Triwrap* m_triangleWrap;  // the inner helper class Triwrap grouping the original TriLib's C functions.

      void* m_in;         // pointers to TriLib's intput, mesh & behavior
      void* m_pmesh;             
      void* m_pbehavior;      
      void* m_vorout;     // pointer to Voronoi output

      float m_minAngle;
      float m_maxArea;
      bool m_convexHullWithSegments;   
      bool m_extraVertexAttr;
      bool m_triangulated;

      std::vector<Point> m_pointList;
      std::vector<int> m_segmentList;
      std::vector<Point> m_holesList;
      std::vector<double> m_defaultExtraAttrs;

   }; // class Delaunay


   //--------------------------------------------
   //
   //  Iterators 
   //   - they allow lazy access to the results
   //
   //--------------------------------------------

   /**
      @brief: The face iterator for a Delaunay triangulation
    */
   class fIterator
   {
   public:
      void operator++();

      fIterator() : m_delaunay(nullptr), meshPointCount(0) { floop.tri = nullptr; };

      bool empty() const;      
      bool isdummy() const;  // pointing to the dummy triangle?

      /**
         @brief: Get the origin point of the triangle

         A triangle abc has origin (Org) a, destination (Dest) b, and apex (Apex) c.
         These vertices occur in counterclockwise order about the triangle.

         @param point: if specified - the cordinates of the vertex
         @return: index of the vertex in the input vector, or -1 if a new vertex was created
       */
      int Org(Delaunay::Point* point = 0) const;
      int Dest(Delaunay::Point* point = 0) const;
      int Apex(Delaunay::Point* point = 0) const;

      /**
         @brief: Get the origin point of the triangle (@see Org() above) and its mesh index

         @param point: the cordinates of the vertex
         @param meshIndex: Index of the vertex in mesh (in order of iteration!)
       */
      void Org(Delaunay::Point& point, int& meshIndex) const;
      void Dest(Delaunay::Point& point, int& meshIndex) const;
      void Apex(Delaunay::Point& point, int& meshIndex) const;

      /**
         @brief: Calculate the area of the triangle
       */
      double area() const;

      // support for iterator dereferencing
      struct Face
      {
         Face(fIterator* iter) : m_iter(iter) {}

         // gets index in the input array
         int Org(Delaunay::Point* point = 0)   const { return m_iter->Org(point); }
         int Dest(Delaunay::Point* point = 0)  const { return m_iter->Dest(point); }
         int Apex(Delaunay::Point* point = 0)  const { return m_iter->Apex(point); }

         // gets index in the resulting mesh
         void Org(Delaunay::Point& point, int& meshIndex)   const { m_iter->Org(point, meshIndex); }
         void Dest(Delaunay::Point& point, int& meshIndex)  const { m_iter->Dest(point, meshIndex); }
         void Apex(Delaunay::Point& point, int& meshIndex)  const { m_iter->Apex(point, meshIndex); }

      private:
         fIterator* m_iter;
      };

      Face operator*() { return Face(this); }

      friend bool operator==(fIterator const&, fIterator const&);
      friend bool operator!=(fIterator const&, fIterator const&);
      friend bool operator<(fIterator const&, fIterator const&); // added mrkkrj

   private:
      struct tdata // TriLib's internal data
      {
         double*** tri;
         int orient;
      };

      typedef struct tdata poface; // = ptr. to oriented face

      fIterator(Delaunay* triangulator);

      int getVertexIndex(/*Triwrap::vertex*/ double* vertexptr) const;
      int getMeshVertexIndex(/*Triwrap::vertex*/ double* vertexptr) const;

      Delaunay* m_delaunay;         
      poface floop;                // TriLib's internal data
      mutable int meshPointCount;  // Used for numbering vertices in a complete triangulation   

      friend struct Face;
      friend class Delaunay;
      friend class TriangulationMesh;
   };
         

   /**
      @brief: This class supports iteration over faces in a foreach() loop
    */
   struct FacesList
   {
      FacesList(Delaunay* triangulator) : m_delaunay(triangulator) {}

      FaceIterator begin();
      FaceIterator end();

   private:
      Delaunay* m_delaunay;
   };


   /**
      @brief: The vertex iterator for a Delaunay triangulation
    */
   class vIterator
   {
   public:
      vIterator operator++();
      Delaunay::Point& operator*() const;

      vIterator() : vloop(nullptr), m_delaunay(nullptr) {}

      int vertexId() const;

      friend class Delaunay;
      friend bool operator==(vIterator const&, vIterator const&);
      friend bool operator!=(vIterator const&, vIterator const&);

   private:
      vIterator(Delaunay* triangulator);   

      Delaunay* m_delaunay;   
      void* vloop;  // TriLib's internal data
   };


   /**
      @brief: The points iterator for a Voronoi tesselation
    */
   class vvIterator 
   {
   public:
      vvIterator operator++();
      Delaunay::Point& operator*() const;

      vvIterator();
      void advance(int steps);

      friend class Delaunay;
      friend bool operator==(vvIterator const&, vvIterator const&);
      friend bool operator!=(vvIterator const&, vvIterator const&);

   private:
      vvIterator(Delaunay* tiangulator);

      Delaunay* m_delaunay;   

      void* vvloop; // TriLib's internal data
      int vvindex;
      int vvcount;
   };


   /**
      @brief: The edges iterator for a Voronoi tesselation
    */
   class veIterator 
   {
   public:    
      veIterator operator++();

      veIterator();

      // OPEN TODO:: comment!
      int startPointId() const;
      int endPointId(Delaunay::Point& normvec) const;

      /**
         @brief: Access the origin vertex (i.e. start point) of a Voronoi edge
         @return: the start point of the edge
       */
      const Delaunay::Point& Org();

      /**
         @brief: Access the destination vertex (i.e. end point) of a Voronoi edge

         @param finiteEdge: true for finite edges, false for inifinte rays
         @return: the end point of the edge, for infinite rays - the *normal vector* of the ray!
       */
      Delaunay::Point Dest(bool& finiteEdge);

      friend class Delaunay;
      friend bool operator==(veIterator const&, veIterator const&);
      friend bool operator!=(veIterator const&, veIterator const&);

   private:
      veIterator(Delaunay* tiangulator);

      Delaunay* m_delaunay;   
      void* veloop;  // TriLib's internal data
      int veindex;
      int vecount;
   };


   /**
      @brief: Class for operations on oriented triangles (faces) of a triangulation mesh
    */
   class TriangulationMesh
   {
   public:
      /**
         @brief: Access the triangle adjoining edge N

         Example:
           Sym(abc, N = 0) -> ba*
           Sym(abc, N = 1) -> cb*
           Sym(abc, N = 2) -> ac*

         Here '*' stands for the farthest vertex on the adjoining triangle whose index is returned

         @param fit: face iterator
         @param i: edge number (N)
         @return: The vertex on the opposite face, or -1 if the edge is part of the convex hull
                  (@see fIterator::Org() above)
       */
      int Sym(FaceIterator const& fit, char i) const;

      /**
         @brief: Access the triangle opposite to current edge of the face

         @param fit: face iterator
         @return: iterator of the opposite face. It is empty if the edge is on the convex hull
       */
      FaceIterator Sym(FaceIterator const& fit) const;

      /**
         @brief: Find the next edge (counterclockwise) of a triangle

         @param fit: face iterator
         @return: iterator corresponding to the next counterclockwise edge of a triangle,
                  Lnext(abc) -> bca
       */
      FaceIterator Lnext(FaceIterator const& fit);

      /**
         @brief: Find the previous edge (clockwise) of a triangle

         @param fit: face iterator
         @return: iterator corresponding to the previous clockwise edge of a triangle,
                  Lprev(abc) -> cab
       */
      FaceIterator Lprev(FaceIterator const& fit);

      /**
         @brief: Find the next edge (counterclockwise) of a triangle with the same origin

         @param fit: face iterator
         @return: iterator corresponding to the next edge counterclockwise with the same origin,
                  Onext(abc) -> ac* (@see Sym() above)
       */
      FaceIterator Onext(FaceIterator const& fit);

      /**
         @brief: Find the next edge clockwise with the same origin

         @param fit: face iterator
         @return: iterator corresponding to the next edge clockwise with the same origin,
                  Oprev(abc) -> a*b (@see Sym() above)
       */
      FaceIterator Oprev(FaceIterator const& fit);

      /**
         @brief: Calculate incident triangles around a vertex

         Note that behaviour is undefined if vertexId is greater than number of vertices - 1.
         All triangles returned have Org(triangle) = vertexId and are in counterclockwise order.

         @param vertexId: the vertex for which you want incident triangles
         @param ivv: triangles around a vertex in counterclockwise order
       */
      void trianglesAroundVertex(int vertexId, std::vector<int>& ivv);

      /**
         @brief:  Point locate a vertex V

         @param vertexId: the vertex
         @return: a face iterator whose origin is V
       */
      FaceIterator locate(int vertexId); // OPEN:: doesn't seem to be working!


      // TODO List: 
      /*  dnext:  Find the next edge counterclockwise with the same destination.   */
      /*  dnext(abc) -> *ba                                                        */
      /*                                                                           */
      /*  dprev:  Find the next edge clockwise with the same destination.          */
      /*  dprev(abc) -> cb*                                                        */
      /*                                                                           */
      /*  rnext:  Find the next edge (counterclockwise) of the adjacent triangle.  */
      /*  rnext(abc) -> *a*                                                        */
      /*                                                                           */
      /*  rprev:  Find the previous edge (clockwise) of the adjacent triangle.     */
      /*  rprev(abc) -> b**                                                        */

   private:
      TriangulationMesh(Delaunay* triangulator);

      Delaunay* m_delaunay;

      friend class Delaunay;
   };

} // namespace tpp

#endif
