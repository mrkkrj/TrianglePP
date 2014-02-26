/*! \file tpp_impl.cpp
    \brief The implementation of the 2D delaunay triangulation class.

	This class is a wrapper on the triangle package.
 */

#include <iostream>
// changed mrkkrj --
//#include <triangle_impl.hpp>
//#include <tpp_interface.hpp>
#define NO_TIMER
#define DREDUCED
#define ANSI_DECLARATORS
#define TRILIBRARY
//#define CDT_ONLY // (de)activate -q option!
#define CPU86

// mrkkrj::: DEBUG trace 
//	- needed when debugging on Windows without console
//#define DBG_TO_FILE 1
#ifdef DBG_TO_FILE
#	include <cstdio>
	FILE* g_debugFile = 0;
#	define TRACE(a) { fprintf(g_debugFile, "%s\n", a); fflush(g_debugFile); }
#	define TRACE2i(a,b) { fprintf(g_debugFile, "%s%d\n", a, b); fflush(g_debugFile); }
#	define TRACE2s(a,b) { fprintf(g_debugFile, "%s%s\n", a, b); fflush(g_debugFile); }
#	define TRACE2b(a,b) { fprintf(g_debugFile, "%s%s\n", a, b?"true ":"false"); fflush(g_debugFile); }
#	define INIT_TRACE(a) { g_debugFile = fopen(a, "w"); \
						   if(!g_debugFile) std::cerr << "ERROR: Cannot open trace file: " << a << std::endl; }
#	define END_TRACE() { fclose(g_debugFile); }
#else
#	define TRACE(a) 
#	define TRACE2i(a,b) 
#	define TRACE2s(a,b) 
#	define TRACE2b(a,b) 
#	define INIT_TRACE(a) 
#	define END_TRACE() 
#endif

#define DETAIL_DEBUG_TRIANGLE 0
// end DEBUG trace

#include "triangle_impl.hpp"
#include "tpp_interface.hpp"
// END changed --

#include <new>

#define REAL double


namespace tpp {

using std::cout;
using std::cerr;


/*!
  Triangulate the points stored in m_PList.
  \note (mrkkrj) Copy-pasted from parts of the original Triangle's triangulate() function!
  \author Piyush Kumar
*/
void Delaunay::Triangulate(std::string& triswitches){
	typedef struct triangulateio  TriangStruct;
	typedef struct triangulateio* pTriangStruct;

	INIT_TRACE("triangle.out.txt");
	TRACE("Triangulate ->");

#if DETAIL_DEBUG_TRIANGLE
    size_t posV = triswitches.find("V");
    if(posV != std::string::npos){
        triswitches.insert(posV, "V"); // detailed trace!
    }
#endif

	m_in = new TriangStruct;
	pTriangStruct pin = (struct triangulateio *)m_in;
	
	pin->numberofpoints = (int)m_PList.size();
  	pin->numberofpointattributes = (int)0;
	pin->pointlist = static_cast<double *>((void *)(&m_PList[0])) ;
  	pin->pointattributelist = NULL;
  	pin->pointmarkerlist = (int *) NULL;
 	pin->numberofsegments = 0;
  	pin->numberofholes = 0;
  	pin->numberofregions = 0;
  	pin->regionlist = (REAL *) NULL;

	m_delclass = new Triwrap;
	Triwrap *pdelclass = (Triwrap *)m_delclass;
	triswitches.push_back('\0');
	char *ptris = &triswitches[0];

	m_pmesh = new Triwrap::__pmesh;
	m_pbehavior = new Triwrap::__pbehavior;

	Triwrap::__pmesh     * tpmesh     = (Triwrap::__pmesh *)     m_pmesh;
	Triwrap::__pbehavior * tpbehavior = (Triwrap::__pbehavior *) m_pbehavior;

	// parse the options:
	pdelclass->parsecommandline(1, &ptris, tpbehavior);

	// initialize data structs
	pdelclass->triangleinit(tpmesh);
	tpmesh->steinerleft = tpbehavior->steiner; // added mrkkrj

	pdelclass->transfernodes(
			    tpmesh, tpbehavior, pin->pointlist, 
				pin->pointattributelist,
                pin->pointmarkerlist, pin->numberofpoints,
                pin->numberofpointattributes);

	// do it!
	tpmesh->hullsize = pdelclass->delaunay(tpmesh, tpbehavior);

	// OPEN TODO:: 
	//	if(concave hull) - compute concave hull wuth the chi-algorithm,
	//					 - use it as segments in formskeleton()!!
	// end TODO::

  /* Ensure that no vertex can be mistaken for a triangular bounding */
  /*   box vertex in insertvertex().                                 */
  	tpmesh->infvertex1 = (Triwrap::vertex) NULL;
  	tpmesh->infvertex2 = (Triwrap::vertex) NULL;
  	tpmesh->infvertex3 = (Triwrap::vertex) NULL;

	// added mrkkrj: support for the -q option
	if (tpbehavior->usesegments && (tpmesh->triangles.items > 0)) {
		tpmesh->checksegments = 1;          /* Segments will be introduced next. */
		if (!tpbehavior->refine) {
		  /* Insert PSLG segments and/or convex hull segments. */
		  pdelclass->formskeleton(tpmesh, tpbehavior, pin->segmentlist,
								  pin->segmentmarkerlist, pin->numberofsegments);
		}
	}

#ifndef CDT_ONLY 
	if (tpbehavior->quality && (tpmesh->triangles.items > 0)) {
		pdelclass->enforcequality(tpmesh, tpbehavior);		/* Enforce angle and area constraints. */
	}
#endif /* not CDT_ONLY */

//#if 0 -> mrkkrj
	if (tpbehavior->poly && (tpmesh->triangles.items > 0)) {
		tpmesh->holes = 0;
		tpmesh->regions = 0;

		if (!tpbehavior->refine) {
		  /* Carve out holes and concavities. */			
		  pdelclass->carveholes(tpmesh, tpbehavior, NULL, tpmesh->holes, NULL, tpmesh->regions);
		}
	} 
//#endif

	/* Calculate the number of edges. */
	tpmesh->edges = (3l * tpmesh->triangles.items + tpmesh->hullsize) / 2l;
	pdelclass->numbernodes(tpmesh, tpbehavior);
	TRACE2i("<- Triangulate: triangles= ", tpmesh->triangles.items);

	m_Triangulated = true;
	END_TRACE();
}

/*!
*/
Delaunay::~Delaunay(){
        struct triangulateio *pin = (struct triangulateio *)m_in;

	Triwrap *pdelclass =  (Triwrap *)m_delclass;

    Triwrap::__pmesh     * tpmesh     = (Triwrap::__pmesh *)     m_pmesh;
    Triwrap::__pbehavior * tpbehavior = (Triwrap::__pbehavior *) m_pbehavior;

	pdelclass->triangledeinit(tpmesh, tpbehavior);

	delete tpmesh;
	delete tpbehavior;
	delete pin;
	delete pdelclass;	
}

/*!
*/
void Delaunay::writeoff(std::string& fname){
	if(!m_Triangulated) {
		cerr << "FATAL: Write called before triangulation\n";
		exit(1);
	}

    Triwrap::__pmesh     * tpmesh     = (Triwrap::__pmesh *)     m_pmesh;
    Triwrap::__pbehavior * tpbehavior = (Triwrap::__pbehavior *) m_pbehavior;

	Triwrap *pdelclass =  (Triwrap *)m_delclass;
	char *pfname = new char[fname.size()+1];
	strcpy(pfname , fname.c_str());

	pdelclass->writeoff(tpmesh, tpbehavior, pfname, 0, NULL);
	delete [] pfname;
}

/*!
*/
int Delaunay::nedges(){
	Triwrap::__pmesh     * tpmesh     = (Triwrap::__pmesh *)     m_pmesh;
	return tpmesh->edges;
}

/*!
*/
int Delaunay::ntriangles(){
	Triwrap::__pmesh     * tpmesh     = (Triwrap::__pmesh *)     m_pmesh;
	return tpmesh->triangles.items;	
}

/*!
*/
int Delaunay::nvertices(){
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
int Delaunay::hull_size(){
	Triwrap::__pmesh     * tpmesh     = (Triwrap::__pmesh *)     m_pmesh;
	return  tpmesh->hullsize;	
}

/*!
*/
int Delaunay::vertexId(vIterator const &vit){
	Triwrap::__pmesh     * tpmesh     = (Triwrap::__pmesh *) vit.MyDelaunay->m_pmesh;
	return ((int *)vit.vloop)[tpmesh->vertexmarkindex];
}


///////////////////////////////
//
// Vertex Iterator Impl.
//
///////////////////////////////

/*!
*/
Delaunay::vIterator::vIterator(Delaunay* adel) {
     typedef Triwrap::vertex vertex;
     MyDelaunay = adel;

     Triwrap::__pmesh     * tpmesh     = (Triwrap::__pmesh *) adel->m_pmesh;
     Triwrap::__pbehavior * tpbehavior = (Triwrap::__pbehavior *) adel->m_pbehavior;
     Triwrap *pdelclass =  (Triwrap *)adel->m_delclass;

     pdelclass->traversalinit(&( tpmesh->vertices ) );
     vloop = pdelclass->vertextraverse(tpmesh);

     while
    	(
		 tpbehavior->jettison || 
	   	(
		  ((int *)vloop)[tpmesh->vertexmarkindex+1] == UNDEADVERTEX
		)
	) 
	vloop = (void *) pdelclass->vertextraverse(tpmesh);
}

/*!
*/
Delaunay::vIterator::~vIterator(){
}

/*!
*/
Delaunay::vIterator Delaunay::vend(){
	vIterator vit;
	vit.vloop = ((Triwrap::vertex) NULL);
	return vit;
}

/*!
*/
Delaunay::vIterator Delaunay::vIterator::operator++() {
     typedef Triwrap::vertex vertex;	

     Triwrap::__pmesh     * tpmesh     = (Triwrap::__pmesh *) MyDelaunay->m_pmesh;
     Triwrap::__pbehavior * tpbehavior = 
				(Triwrap::__pbehavior *) MyDelaunay->m_pbehavior;

     Triwrap *pdelclass =  (Triwrap *) MyDelaunay->m_delclass;

     while
        (
             tpbehavior->jettison ||
            (
              ((int *)vloop)[tpmesh->vertexmarkindex+1] == UNDEADVERTEX
            )
        )
        vloop = (void *) pdelclass->vertextraverse(tpmesh);
        vloop = (void *) pdelclass->vertextraverse(tpmesh);

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
Delaunay::fIterator::fIterator(Delaunay* adel) {
     typedef Triwrap::vertex vertex;
     typedef Triwrap::__otriangle trianglelooptype;

     MyDelaunay = adel;

     Triwrap::__pmesh     * tpmesh     = (Triwrap::__pmesh *) adel->m_pmesh;
     Triwrap *pdelclass =  (Triwrap *)adel->m_delclass;

     pdelclass->traversalinit(&( tpmesh->triangles ) );
     // floop = new trianglelooptype;
     trianglelooptype *ploop = (trianglelooptype *)(&floop);
     ploop->tri    = pdelclass->triangletraverse(tpmesh);
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
    fit.floop.tri = (double ***) NULL;
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
     Triwrap *pdelclass =  (Triwrap *) MyDelaunay->m_delclass;

     ploop->tri = pdelclass->triangletraverse(tpmesh);
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

	 return ((unsigned)ret < m_PList.size()) ? ret : -1;
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
	 
	 retval.floop.tri = NULL;
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
	Triwrap  *pdelclass               = (Triwrap *)              m_delclass;

	horiz.tri = tpmesh->dummytri;
    horiz.orient = 0;
	symself(horiz);
	double dv[2];
	dv[0] = m_PList[vertexid][0];
	dv[1] = m_PList[vertexid][1];

	/* Search for a triangle containing `newvertex'. */
	int intersect = pdelclass->locate(tpmesh, tpbehavior, dv, &horiz);
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

} // namespace tpp ends.
