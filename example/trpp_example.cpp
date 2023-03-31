/** 
    @file   trpp_example.cpp
    @brief  example usage of the Triangle++ wrapper
 */

#include "tpp_interface.hpp"

#include <vector>
#include <iostream>
#include <cassert>

using namespace tpp;
using Point = Delaunay::Point;


int main()
{
    // prepare input
    std::vector<Point> delaunayInput;
    
    delaunayInput.push_back(Point(0,0));
    delaunayInput.push_back(Point(1,1));
    delaunayInput.push_back(Point(0,2));
    delaunayInput.push_back(Point(3,3));

    // 1. standard triangulation
    Delaunay trGenerator(delaunayInput);
    trGenerator.Triangulate();

    // iterate over triangles
    for (FaceIterator fit = trGenerator.fbegin(); fit != trGenerator.fend(); ++fit)
    {
        int keypointIdx1 = fit.Org();
        int keypointIdx2 = fit.Dest();
        int keypointIdx3 = fit.Apex();

        // access data
        double x1 = delaunayInput[keypointIdx1][0];
        double y1 = delaunayInput[keypointIdx1][1];
    }
    
    std::cout << " Example: 1";

    // 2. triangulation with quality constraints
    bool enforceQuality = true;

    // set custom constraints
    //  - if nothing set, the default constraint is minAngle = 20.0 deg
    trGenerator.setMinAngle(30.5f);
    trGenerator.setMaxArea(1.5f);

    trGenerator.Triangulate(enforceQuality);

    // iterate over triangles
    for (FaceIterator fit = trGenerator.fbegin(); fit != trGenerator.fend(); ++fit)
    {
       // potential Steiner points:
       Point sp1, sp2, sp3;

       int keypointIdx1 = fit.Org(&sp1);
       int keypointIdx2 = fit.Dest(&sp2);
       int keypointIdx3 = fit.Apex(&sp3);

       // new vertices might have been added to enforce constraints!
       //  (i.e. Steiner points)
       if (keypointIdx1 == -1)
       {
          double x1 = sp1[0]; // an added vertex, it's data copied to sp1
          double y1 = sp1[1];
       }
       else
       {
          // a point from original data
          double x1 = delaunayInput[keypointIdx1][0];
          double y1 = delaunayInput[keypointIdx1][1];

          // but that will work too!
          x1 = sp1[0];
          y1 = sp1[1];
       }
    }

    std::cout << ", 2";

    // 3. creation of a Voronoi diagram
    trGenerator.Tesselate();

    auto vPointCount = trGenerator.voronoiPointCount(); // how many?
    auto vEdgesCount = trGenerator.voronoiEdgeCount();

    // iterate over Voronoi points
    for (VoronoiVertexIterator vit = trGenerator.vvbegin(); vit != trGenerator.vvend(); ++vit)
    {
       // access data
       auto point = *vit;
       double x1 = point[0];
       double y1 = point[1];
    }

    std::cout << ", 3a";

    // and Voronoi edges
    bool infiniteRay = false;

    for (VoronoiEdgeIterator eit = trGenerator.vebegin(); eit != trGenerator.veend(); ++eit)
    {       
       Point p1 = eit.Org();
       Point p2 = eit.Dest(infiniteRay);

       // access data
       double xstart = p1[0];
       double ystart = p1[1];

       if(infiniteRay)
       {
          // an inifinite ray, thus no endpoint coordinates!
          auto rayNormalXValue = p2[0];
          auto rayNormalYValue = p2[1];
          assert(!(rayNormalXValue == 0.0 && rayNormalYValue == 0.0));
       }
       else
       {
          double xend = p2[0];
          double yend = p2[1];
       }
    }

    std::cout << ", 3b";

    // 4. constrained Delaunay
    std::vector<Point> segments;

    segments.push_back(delaunayInput[2]);
    segments.push_back(delaunayInput[1]);

    trGenerator.setSegmentConstraint(segments);
    // trGenerator.setHolesConstraint(holes); --> also supported!

    trGenerator.Triangulate(!enforceQuality);   // no quality constraint, thus NO Steiner points!
    int triCount = trGenerator.triangleCount(); // how many?

    // iterate over triangles
    for (FaceIterator fit = trGenerator.fbegin(); fit != trGenerator.fend(); ++fit)
    {
       int keypointIdx1 = fit.Org();
       int keypointIdx2 = fit.Dest();
       int keypointIdx3 = fit.Apex();

       // access data
       double x1 = delaunayInput[keypointIdx1][0];
       double y1 = delaunayInput[keypointIdx1][1];
    }

    std::cout << ", 4 --> OK! \n";
}

// --- eof ---
