
/*! \file tpp_interface.hpp
    \brief The main Delaunay C++ class of the Triangle++ wrapper.

    Use this class to produce Delaunay triangulations.

    The following description pertains to the original version, the current version
    was ported to VisualStudio. Thus it doesn't need Python scripts, and is supposed
    to be used *as it is* in your program!
 */


 /*! \mainpage Triangle++
 \section intro Introduction
 <table border="0">
 <tr><td>
 If you do not know, what a Delaunay triangulation is, you can read more about it
 <a href="http://www.compgeom.com/~piyush/teach/5930/slides/lecture8.ppt">here</a> and
 <a href="http://en.wikipedia.org/wiki/Delaunay_triangulation">here</a>.
 This C++ library module is just a wrapper class on the
 <a href="http://www.cs.berkeley.edu/~jrs/">Triangle</a>
 package of <a href="http://www.cs.berkeley.edu/~jrs/">Jonathan</a>.

 Many times I have had to use triangle in C++ code bases of mine and have been forced to use C.
 At last I thought I would put a wrapper on his cool C code and it seems that this is what I got.

 The design is not perfect and the code was written in a day, but it does compile and run on the
 machines I tried (cygwin/redhat). The C++ wrapper will certainly slow access down if you want to
 mess with the triangulation but the basic delaunay triangulation should be as fast as triangle.

 Look at the tpp_interface.hpp file for getting started on what this wrapper can do for you. Also
 have a look at main.cpp which shows an example of using this class. The class is thread-safe.
 <p>
 <b>Requirements</b> : Python, make and C++ compilers.
 Supported C/C++ Compilers: g++ / icpc (Intel C++).
 Also needed is doxygen for generating documentation.
 </p>
 <p>
 <b>Compilation</b> : Just type 'make'</p>
 <p>
 <b>Testing</b> : Goto the bin directory, and type './test ../data/input.dat' (after compilation of course).
 </p>
 </td>
 <td><img src="http://upload.wikimedia.org/wikipedia/en/9/92/Delaunay_triangulation.png" alt="Delaunay Triangulation Example"></td>
 </tr>
 </table>

 \section Downloads
 You can download the latest version of the source code from <a href="triangle++.tar.gz">here</a>.

 \section authors Authors
     <ul>
         <li><a href="http://compgeom.com/~piyush">Piyush Kumar</a></li>
         <li><a href="http://www.ib-krajewski.de">Marek Krajewski</a></li>
         <li>Hopefully more to come... (please feel free to extend this wrapper)</li>
     </ul>

 \section changelog Change Log

 17/04/20: mrkkrj – added support Voronoi tesselation <br>
 22/01/20: mrkkrj – added support for custom constraints (angle and area) <br>
 17/09/18: mrkkrj – ported to 64-bit (preliminary, not thorougly tested!) <br>
 11/07/11: mrkkrj - bugfix in Triangle's divandconqdelauney() <br>
 10/15/11: mrkkrj - added support for the "quality triangulation" option, added some debug support<br>
 08/24/11: mrkkrj - Ported to VisualStudio, added comp. operators, reformatted and added some comments<br>
 10/21/06: Replaced vertexsort with C++ sort.<br>
 10/25/06: Wrapped in tpp namespace for usage with other libraries with similar names.
           Added some more documentation/small changes. Used doxygen 1.5.0 and dot. Tested compilation with
           icc 9.0/9.1, gcc-4.1/3.4.6. <br>
 11/03/06: Fixed the compilation system. <br>

 \todo
 <ol>
     <li> Intel Compiler Warnings with -Wall </li>
     <ul>
          <li> remove the compiler warnings for icpc 9.0/9.1</li>
     </ul>

     <li> Implement vertexmedian() in C++. </li>
     <li> Implement the flip operator as a member function of Delaunay. </li>
 </ol>
 */

 //-----------------------------------------------------------

#ifndef TRPP_INTERFACE
#define TRPP_INTERFACE

// changed mrkkrj --
//#include <dpoint.hpp>
#include "dpoint.hpp"
// END changed --
#include <vector>
#include <string>

//! The main namespace in which the Triangle++ project lives
namespace tpp {

   // (mrkkrj)
   enum DebugOutputLevel {
      None, 
      Info,    // most useful; it gives information on algorithmic progress and much more detailed statistics
      Vertex,  // gives vertex-by-vertex details, and prints so much that Triangle runs much more slowly
      Debug    // gives information only a debugger could love
   };

   //!  The main Delaunay Class that wraps around Triangle.
   /*!
     This is a C++ wrapper of the Triangle package by JRS.

     This class currently uses the dpoint class written by me (the point class is a d-dimensional point
     class reviver::dpoint (but for this application it only uses the d=2 case).
     Additionally, the inner helper C++ class Triwrap groups the original Triangle's C functions.

     \author Piyush Kumar, mrkkrj

     \note   (mrkkrj) For for backgroud info on the Triangle's implementation see "Triangle:
             Engineering a 2D Quality Mesh Generator and Delaunay Triangulator" by JP Shewchuk:
             www.cs.cmu.edu/~quake-papers/triangle.ps
   */
   class Delaunay {
   public:
      //! Point Typedef
      /*! Warning: If you want to use your own point class, you might have to
          work hard...
           - mrkkrj: true!!! - spare your time, use an adapter class.
      */
      typedef reviver::dpoint <double, 2> Point;

      //! The main constructor.
      /*!
        Takes a vector of 2 dimensional points where each of the coordinates is
        expressed as double.
      */
      Delaunay(std::vector<Point>& v);

      //! The main destructor.
      /*!
        Does memory cleanup mostly.
      */
      ~Delaunay();

      //! Delaunay Triangulate the input points (Constrained)
      /*!
        This function calls Triangle.h to Delaunay-triangulate points given as input to the 
        constructor of this class. Here a Constrained triangualtion will be created.

        \param quality enforce minimal angle (default: 20°) and, minimal area (only if explicitely set)
      */
      void Triangulate(bool quality = false, DebugOutputLevel = None);

      //! Delaunay Triangulate the input points (Conforming)
      /*!
        This function calls Triangle.h to Delaunay-triangulate points given as input to the
        constructor of this class. Here a Conforming triangualtion will be created.
      */
      void TriangulateConf(DebugOutputLevel = None);

      //! Voronoi-tesselate the input points (added  mrkkrj)
      /*!
        This function calls triangle to create a Voronoi diagram with points given as input
        to the constructor of this class.

        Note that a Voronoi diagram can be only created if the underlying triangulation is convex 
        and doesn't have holes!

        \param useConformingDelaunay use conforming Delaunay triangulation as base for the Voronoi diagram
      */
      void Tesselate(bool useConformingDelaunay = false, DebugOutputLevel traceLvl = None);

      //! Set a quality constraint for the triangulation
      /*!
        \param angle min. resulting angle, if angle <= 0, the constraint will be removed.
      */
      void setMinAngle(float angle) {
         m_minAngle = angle;
      }

      //! Set a quality constraint for the triangulation
      /*!
        \param area max. triangle area, if area <= 0, the constraint will be removed.
      */
      void setMaxArea(float area) {
         m_maxArea = area;
      }

      //! Are the constrainst sane?
      /*!
        \possible true if is highly probable for triangualtion to succeed
        \return true if triangualtion is guaranteed to succeed
      */
      bool checkConstraints(bool& possible) const;

      //! Are the constrainst sane, take two
      /*!
        \relaxed report highly probable as correct too, as error otherwise
        \return true if triangualtion is guaranteed to succeed, or at least higly probable to
      */
      bool checkConstraintsOpt(bool relaxed) const;

      //! Get minAngle intervals
      /*!
        \guaranteed up to this value triangualtion is guaranteed to succeed
        \possible up to this value it is highly probable for triangualtion to succeed
      */
      static void getMinAngleBoundaries(float& guaranteed, float& possible);

      //! Set a user test function for the triangulation
      /*!
        OPEN TODO::: (use the -u switch!!!!)
      */
      void setUserConstraint(bool (*f)()) {};

      //! Output a geomview .off file containing the delaunay triangulation
      /*!
        \param fname output file name.
      */
      void writeoff(std::string& fname);

      //! Number of edges in the triangulation
      /*!
        \return Number of Edges
        Remember to call Triangulate before using this function.
      */
      int nedges() const;

      //! Number of triangles in the triangulation
      /*!
        \return Number of Triangles
        Remember to call Triangulate before using this function.
      */
      int ntriangles() const;

      //! Number of vertices in the triangulation
      /*!
        \return Number of Vertices
        Remember to call Triangulate before using this function.
      */
      int nvertices() const;

      //! Number of vertices on the convex hull.
      /*!
        \return Number of vertices on the convex hull.
        Remember to call Triangulate before using this function.
      */
      int hull_size() const;

      //! Number of Voronoi points in the tesselation
      /*!
        \return Number of Points
        Remember to call Tesselate before using this function.
      */
      int nvpoints() const;

      //! Number of Voronoi edges in the tesselation
      /*!
        \return Number of Edges
        Remember to call Tesselate before using this function.
      */
      int nvedges() const;


      ///////////////////////////////
      //
      // Vertex Iterator
      //
      ///////////////////////////////

      //!  The vertex iterator for the Delaunay class
      class vIterator {
      private:
         vIterator(Delaunay* tiangulator);   //! To set container

         Delaunay* MyDelaunay;   //! Which container do I point
         void* vloop;            //! Triangles Internal data.

      public:
         vIterator operator++();
         vIterator() :vloop(nullptr) {};
         Point& operator*() const;
         ~vIterator();

         friend class Delaunay;
         friend bool operator==(vIterator const&, vIterator const&);
         friend bool operator!=(vIterator const&, vIterator const&);
      };

      //! Vertex iterator begin function
      vIterator vbegin() { return vIterator(this); };
      //! Vertex iterator end function
      vIterator vend();

      //! Given an iterator, find its index in the input vector of points.
      int vertexId(vIterator const& vit) const;

      //! Given an index, return the actual double Point
      const Point& point_at_vertex_id(int i) { return m_PList[i]; };

      //! Return the Point additionally created in quality mesh generation ("q" option)
      Point added_point_at_vertex_id(int i);

      friend class vIterator;


      ///////////////////////////////
      //
      // Face Iterator
      //
      ///////////////////////////////

      //!  The face iterator for the Delaunay class
      class fIterator {
      private:

         struct tdata {
            double*** tri;
            int orient;
         };

         typedef struct tdata  poface;

         fIterator(Delaunay* tiangulator);  //! To set container

         Delaunay* MyDelaunay;   //! Which container do I point
         //void *floop;          //! Triangles Internal data.
         poface floop;

      public:
         void operator++();
         fIterator() { floop.tri = nullptr; };
         ~fIterator();

         friend class Delaunay;
         friend bool operator==(fIterator const&, fIterator const&);
         friend bool operator!=(fIterator const&, fIterator const&);
         friend bool operator<(fIterator const&, fIterator const&); // added mrkkrj
      };

      //! Face iterator begin function
      fIterator fbegin() { return fIterator(this); };
      //! Face iterator end function
      fIterator fend();
      int faceId(fIterator const&);


      //! Access the origin (Org) vertex of a face.
      /*!
        \param fit  Face interator.
        \param point  if specified: the cordinates of the vertex
        \return Index of the vertex in m_pList,
                or -1 if quality option was used and a new vertex was created!

        A triangle abc has origin (org) a,destination (dest) b, and apex (apex)
        c.  These vertices occur in counterclockwise order about the triangle.
        Remember to call Triangulate before using this function. Do not use it on a null iterator.
      */
      int Org(fIterator const& fit, Point* point = 0);


      //! Access the destination (Dest) vertex of a face.
      /*!
        \param fit  Face interator.
        \param point  if specified: the cordinates of the vertex
        \return Index of the vertex in m_pList,
                or -1 if quality option was used and a new vertex was created!

        A triangle abc has origin (org) a,destination (dest) b, and apex (apex)
        c.  These vertices occur in counterclockwise order about the triangle.
        Remember to call Triangulate before using this function. Do not use it on a null iterator.
      */
      int Dest(fIterator const& fit, Point* point = 0);


      //! Access the apex (Apex) vertex of a face.
      /*!
        \param fit  Face interator.
        \param point  if specified: the cordinates of the vertex
        \return Index of the vertex in m_pList,
                or -1 if quality option was used and a new vertex was created!

        A triangle abc has origin (org) a,destination (dest) b, and apex (apex)
        c.  These vertices occur in counterclockwise order about the triangle.
        Remember to call Triangulate before using this function. Do not use it on a null iterator.
      */
      int Apex(fIterator const& fit, Point* point = 0);


      //! Access the triangle adjoining edge i
      /*!
        \param fit  Face Iterator
        \param i  edge number
        \return The vertex on the opposite face, or -1 (see Org() above)

        A triangle abc has origin (org) a,destination (dest) b, and apex (apex)
            c.  These vertices occur in counterclockwise order about the triangle.
        <ul>
        <li>sym(abc, 0) -> ba*</li>
        <li>sym(abc, 1) -> cb*</li>
        <li>sym(abc, 2) -> ac*</li>
        </ul>
        * is the farthest vertex on the adjoining triangle whose index
        is returned. A -1 is returned if the edge is part of the convex hull.
        Remember to call Triangulate before using this function.
        Do not use it on a null iterator.
      */
      int Sym(fIterator const& fit, char i);


      //! Access the triangle opposite to current edge of the face
      /*!
        \param fit  Face iterator
        \return The iterator of the opposite face

        A triangle abc has origin (org) a,destination (dest) b, and apex (apex)
            c.  These vertices occur in counterclockwise order about the triangle.
        The iterator
        to the triangle is returned. The iterator is empty if the edge
        is on the convex hull.
            Remember to call Triangulate before using this function.
        Do not use it on a null iterator.
      */
      fIterator Sym(fIterator const& fit);


      //! Is the iterator empty?
      /*!
        \param fit  Face interator.
        \return true if the iterator is empty
      */
      inline bool empty(fIterator const& fit)
      {
         return fit.floop.tri == nullptr;
      };


      //! Is the iterator pointing to the dummy triangle?
      /*!
        \param fit  Face interator.
        \return true if the iterator is of the dummy triangle.
      */
      bool isdummy(fIterator const& fit);


      //! Find the next edge (counterclockwise) of a triangle.   
      /*!
        \param fit  face iterator
        \return The face iterator corresponding to the next counterclockwise edge of a triangle

        Lnext(abc) -> bca.
        Remember to call Triangulate before using this function.
        Do not use it on a null iterator.
      */
      fIterator Lnext(fIterator const& fit);


      //! Find the previous edge (clockwise) of a triangle.   
      /*!
        \param fit  face iterator
        \return The face iterator corresponding to the previous clockwise edge of a triangle

        Lprev(abc) -> cab.
        Remember to call Triangulate before using this function.
        Do not use it on a null iterator.
      */
      fIterator Lprev(fIterator const& fit);


      //! Find the next edge (counterclockwise) of a triangle with the same origin
      /*!
        \param fit  face iterator
        \return The face iterator corresponding to the next edge counterclockwise with the same origin.

        Onext(abc) -> ac*.
        Remember to call Triangulate before using this function.
        Do not use it on a null iterator.
      */
      fIterator Onext(fIterator const& fit);


      //! Find the next edge clockwise with the same origin.
      /*!
        \param fit  face iterator
        \return The face iterator corresponding to the next edge clockwise with the same origin.

        Onext(abc) -> a*b.
        Remember to call Triangulate before using this function.
        Do not use it on a null iterator.
      */
      fIterator Oprev(fIterator const& fit);


      // TODO List: (for face iterators)
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


      //! Calculate incident triangles around a vertex.
      /*!
      \param vertexid The vertex for which you want incident triangles.
            \param ivv Returns triangles around a vertex in counterclockwise order.

      Note that behaviour is undefined if vertexid is greater than
      number of vertices - 1. Remember to call Triangulate before using this function.
      All triangles returned have Org(triangle) = vertexid.
      All triangles returned are in counterclockwise order.
      */
      void trianglesAroundVertex(int vertexid, std::vector<int>& ivv);


      //! Calculate the area of a face. 
      /*!
        \param fit  Face interator.
        \return area of the face associated with the iterator.

      */
      double area(fIterator const& fit);


      //! Point locate a vertex v
      /*!
        \param vertexid  vertex id
        \return a face iterator whose origin is v.
      */
      fIterator locate(int vertexid); // OPEN:: doesn't seem to be working!


      ///////////////////////////////
      //
      // Voronoi Points Iterator 
      //  (added mrkkrj)
      //
      ///////////////////////////////

      //!  The Voronoi points iterator for the Delaunay class
      class vvIterator {
      public:
         vvIterator();
         vvIterator operator++();
         Point& operator*() const;
         void advance(int steps);

      private:
         vvIterator(Delaunay* tiangulator);   //! To set container

         Delaunay* m_delaunay;    //! Which container do I point to
         void* vvloop;            //! Triangle's Internal data.
         int vvindex;
         int vvcount;

         friend class Delaunay;
         friend bool operator==(vvIterator const&, vvIterator const&);
         friend bool operator!=(vvIterator const&, vvIterator const&);
      };

      //! Voronoi Points iterator begin function
      vvIterator vvbegin() { return vvIterator(this); };
      //! Voronoi Points iterator end function
      vvIterator vvend();


      ///////////////////////////////
      //
      // Voronoi Edges Iterator 
      //  (added mrkkrj)
      //
      ///////////////////////////////

      //!  The Voronoi edges iterator for the Delaunay class
      class veIterator {
      public:
         veIterator();
         veIterator operator++();
         int startPointId() const;
         int endPointId(Point& normvec) const;

      private:
         veIterator(Delaunay* tiangulator);   //! To set container

         Delaunay* m_delaunay;   //! Which container do I point to
         void* veloop;           //! Triangle's Internal data.
         int veindex;
         int vecount;

         friend class Delaunay;
         friend bool operator==(veIterator const&, veIterator const&);
         friend bool operator!=(veIterator const&, veIterator const&);
      };

      //! Voronoi Points iterator begin function
      veIterator vebegin() { return veIterator(this); };
      //! Voronoi Points iterator end function
      veIterator veend();


      //! Access the origin (Org) vertex of an edge. (added mrkkrj)
      /*!
        \param eit  Voronoi Edge iterator.
        \return The start point of the Voronoi edge,

        Remember to call Tesselate before using this function. Do not use it on a null iterator.
      */
      const Point& Org(veIterator const& eit);


      //! Access the destination (Dest) vertex of an edge. (added mrkkrj)
      /*!
        \param eit  Voronoi Edge iterator.
        \param finiteEdge true for finite edges, false for inifinte rays.
        \return The end point of the Voronoi edge, for infinite rays the normal vector of the ray

        Remember to call Tesselate before using this function. Do not use it on a null iterator.
      */
      Point Dest(veIterator const& eit, bool& finiteEdge);


      //--------------------------------------
      // added mrkkrj - helper for Points 
      //    OPEN:: compiler cannot instantiate less<> with operator<() for Point, why?!
      //--------------------------------------
      struct OrderPoints
      {
         bool operator() (const Point& lhs, const Point& rhs) const {
            // first sort on x, then on y coordinates
            if (lhs[0] < rhs[0]) {
               return true;
            }
            if (lhs[0] == rhs[0] && lhs[1] < rhs[1]) {
               return true;
            }
            return false;
         }
      };

   private:
      void Triangulate(std::string& triswitches);

      // added mrkkrj - helper functions for face iterator access methods 
      //    HACK:: double* as not to export internal impl.
      void SetPoint(Point& point, double* vertexptr);
      int GetVertexIndex(fIterator const& fit, double* vertexptr);
      int GetFirstIndexNumber() const;

      // added mrkkrj 
      std::string formatFloatConstraint(float f) const;
      void setDebugLevelOption(std::string options, DebugOutputLevel traceLvl);
      void freeTriangleDataStructs();

      friend class fIterator;

   private:
      std::vector<Point> m_PList;   /*! Stores the input point list. */
      void* m_in;             /*! Used for intput to triangle  */
      void* m_triangleWrap;   /*! Triangle impl. is wrapped in this pointer. */
      void* m_pmesh;          /*! pointer to triangle mesh */
      void* m_pbehavior;
      bool m_Triangulated;

      // added mrkkrj:
      void* m_vorout;          /*! pointer to Voronoi output */

      // added mrkkrj: quality constraints
      float m_minAngle;
      float m_maxArea;

   }; // Class Delaunay

} // namespace tpp ends.


#endif


