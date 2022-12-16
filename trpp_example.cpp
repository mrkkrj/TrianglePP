/*! \file trpp_example.cpp
    \brief example usage of the Triangle++ wrapper
 */

#include "tpp_interface.hpp"
#include <vector>
#include <cassert>


using namespace tpp;


int main()
{
    // prepare input
    std::vector<Delaunay::Point> delaunayInput;
    
    delaunayInput.push_back(Delaunay::Point(0,0));
    delaunayInput.push_back(Delaunay::Point(1,1));
    delaunayInput.push_back(Delaunay::Point(0,2));
    delaunayInput.push_back(Delaunay::Point(3,3));

    // 1. standard triangulation
    Delaunay trGenerator(delaunayInput);
    trGenerator.Triangulate();

    // iterate over triangles
    for (Delaunay::fIterator fit = trGenerator.fbegin(); fit != trGenerator.fend(); ++fit)
    {
        int keypointIdx1 = trGenerator.Org(fit); 
        int keypointIdx2 = trGenerator.Dest(fit);
        int keypointIdx3 = trGenerator.Apex(fit);

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
    for (Delaunay::fIterator fit = trGenerator.fbegin(); fit != trGenerator.fend(); ++fit)
    {
       Delaunay::Point sp1;
       Delaunay::Point sp2;
       Delaunay::Point sp3;

       int keypointIdx1 = trGenerator.Org(fit, &sp1);
       int keypointIdx2 = trGenerator.Dest(fit, &sp2);
       int keypointIdx3 = trGenerator.Apex(fit, &sp3);

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

    auto vPointCount = trGenerator.nvpoints();
    auto vEdgesCount = trGenerator.nvedges();

    // iterate over Voronoi points
    for (Delaunay::vvIterator fit = trGenerator.vvbegin(); fit != trGenerator.vvend(); ++fit)
    {
       // access data
       auto point = *fit;
       double x1 = point[0];
       double y1 = point[1];
    }

    std::cout << ", 3a";

    // ... and Voronoi edges
    for (Delaunay::veIterator fit = trGenerator.vebegin(); fit != trGenerator.veend(); ++fit)
    {       
       bool infiniteRay = false;
       Delaunay::Point p1 = trGenerator.Org(fit);
       Delaunay::Point p2 = trGenerator.Dest(fit, infiniteRay);

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
    std::vector<Delaunay::Point> segments;

    segments.push_back(delaunayInput[2]);
    segments.push_back(delaunayInput[1]);

    trGenerator.setSegmentConstraint(segments);
    // trGenerator.setHolesConstraint(holes); --> also supported!

    trGenerator.Triangulate(!enforceQuality); // no quality constraint, thus no Steiner points!
    int triCount = trGenerator.ntriangles();

    // iterate over triangles
    for (Delaunay::fIterator fit = trGenerator.fbegin(); fit != trGenerator.fend(); ++fit)
    {
       Delaunay::Point sp1;
       Delaunay::Point sp2;
       Delaunay::Point sp3;

       int keypointIdx1 = trGenerator.Org(fit, &sp1);
       int keypointIdx2 = trGenerator.Dest(fit, &sp2);
       int keypointIdx3 = trGenerator.Apex(fit, &sp3);

       // access data
       double x1 = delaunayInput[keypointIdx1][0];
       double y1 = delaunayInput[keypointIdx1][1];
    }

    std::cout << ", 4\n";

    std::cout << " ### all TrianglePP examples finished\n";
}

// --- eof ---
