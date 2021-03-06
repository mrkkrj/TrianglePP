/*! \file trpp_example.cpp
    \brief example usage of the Triangle++ wrapper
 */

#include "tpp_interface.hpp"

#include <vector>
#include <iostream>
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
    delaunayInput.push_back(Delaunay::Point(1.5, 2.125));

    // impl. helper
    auto getTriangulationPt = [&](int keypointIdx, const Delaunay::Point& sPoint, double& x, double& y)
    {
       if (keypointIdx == -1)
       {
          x = sPoint[0]; // added Steiner point, it's data copied to sPoint
          y = sPoint[1];
       }
       else
       {
          // point from original data
          x = delaunayInput[keypointIdx][0];
          y = delaunayInput[keypointIdx][1];
       }
    };

    // impl. helper
    auto checkConstraints = [](const Delaunay& del) -> bool
    {
       bool relaxedTest = true;

       if (!del.checkConstraintsOpt(relaxedTest))
       {
          std::cout << "TEST: constraints out of bounds, skip! \n";
          return false;
       }
       return true;
    };


    // 1. standard triangulation
    std::cout << "TEST: standard triangulation \n";

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

        std::cout << " -- Triangle points: " 
                  << "{" << delaunayInput[keypointIdx1][0] << "," << delaunayInput[keypointIdx1][1] << "}, "
                  << "{" << delaunayInput[keypointIdx2][0] << "," << delaunayInput[keypointIdx2][1] << "}, "
                  << "{" << delaunayInput[keypointIdx3][0] << "," << delaunayInput[keypointIdx3][1] << "}\n";
    }

    // 2. triangulate with quality constraints
    bool withQuality = true;
    tpp::DebugOutputLevel dbgOutput = tpp::Info;
    int nrOfConstraintTests = 3;

    for (int i = 0; i < nrOfConstraintTests; ++i)
    {
       if (i == 0)
       {
          // 2.1
          std::cout << "\nTEST: default constraints (min angle = 20�) \n";
          
          trGenerator.Triangulate(withQuality, dbgOutput);
       }
       else if (i == 1)
       {
          // 2.2
          std::cout << "\nTEST: custom constraints (angle = 30.5�, area = 5.5) \n";
          
          trGenerator.setMinAngle(30.5f);
          trGenerator.setMaxArea(5.5f);

          if (!checkConstraints(trGenerator))
          {
             continue;
          }

          trGenerator.Triangulate(withQuality, dbgOutput);
       }
       else if (i == 2)
       {
          // 2.3
          std::cout << "\nTEST: custom constraints (angle = 44�) \n";
          
          // 44 deg results in an endless loop 
          //  --> triangles too tiny for the floating point precision! 
          trGenerator.setMinAngle(44.0f);
          trGenerator.setMaxArea(-1);

          if (!checkConstraints(trGenerator))
          {
             continue;
          }

          trGenerator.Triangulate(withQuality, dbgOutput);
       }

       // iterate over triangles
       for (Delaunay::fIterator fit = trGenerator.fbegin(); fit != trGenerator.fend(); ++fit)
       {
          Delaunay::Point sp1;
          Delaunay::Point sp2;
          Delaunay::Point sp3;

          int keypointIdx1 = trGenerator.Org(fit, &sp1);
          int keypointIdx2 = trGenerator.Dest(fit, &sp2);
          int keypointIdx3 = trGenerator.Apex(fit, &sp3);

          double x1 = -1; 
          double y1 = -1;
          double x2 = -1;
          double y2 = -1;
          double x3 = -1;
          double y3 = -1;

          getTriangulationPt(keypointIdx1, sp1, x1, y1);
          getTriangulationPt(keypointIdx2, sp2, x2, y2);
          getTriangulationPt(keypointIdx3, sp3, x3, y3);

          std::cout << " -- Triangle points: "
                    << "{" << x1 << "," << y1 << "}, "
                    << "{" << x2 << "," << y2 << "}, "
                    << "{" << x2 << "," << y3 << "}\n";
       }
    }
    
    // 3. Voronoi diagrams
    std::cout << "\nTEST: Voronoi tesselation \n";

    trGenerator.Tesselate();
       
    // iterate over Voronoi points
    for (Delaunay::vvIterator vit = trGenerator.vvbegin(); vit != trGenerator.vvend(); ++vit)
    {
       Delaunay::Point vp = *vit;
       double x1 = vp[0];
       double y1 = vp[1];

       std::cout << " -- Voronoi point: "
          << "{" << x1 << "," << y1 << "}\n";
    }

    // OPEN TODO:::

    // iterate over Voronoi edges !!!!
    // use conforming Delaunay as base !!!!
   
    // OPEN TODO::: ...


    // 4. segment-constrained triangulation
    std::cout << "\nTEST: segment-constrainded triangluation (CDT) \n";

    // prepare input 
    //    - see "example constr segments.jpg" for visualisation!
    std::vector<Delaunay::Point> constrDelaunayInput;
    
    constrDelaunayInput.push_back(Delaunay::Point(0, 0));
    constrDelaunayInput.push_back(Delaunay::Point(0, 1));
    constrDelaunayInput.push_back(Delaunay::Point(0, 3));

    constrDelaunayInput.push_back(Delaunay::Point(2, 0));

    constrDelaunayInput.push_back(Delaunay::Point(4, 1.25));
    constrDelaunayInput.push_back(Delaunay::Point(4, 3));

    constrDelaunayInput.push_back(Delaunay::Point(6, 0));
    constrDelaunayInput.push_back(Delaunay::Point(8, 1.25));

    constrDelaunayInput.push_back(Delaunay::Point(9, 0));
    constrDelaunayInput.push_back(Delaunay::Point(9, 0.75));
    constrDelaunayInput.push_back(Delaunay::Point(9, 3));
   
    // - reference triangulation
    Delaunay trConstrGenerator(constrDelaunayInput);
    trConstrGenerator.Triangulate(withQuality);

    int triangleCt = trConstrGenerator.ntriangles();
   
    //assert(triangleCt == 11);
    std::cout << " -- Unconstrained triangle count: " << triangleCt << "\n";

    // prepare segments 
    //    - see "example constr segments.jpg" for visualisation!
    std::vector<Delaunay::Point> constrDelaunaySegments;

    constrDelaunaySegments.push_back(Delaunay::Point(0, 1));
    constrDelaunaySegments.push_back(Delaunay::Point(9, 0.75));

    // - CDT triangulation

    trConstrGenerator.setSegmentConstraint(constrDelaunaySegments);
    trConstrGenerator.Triangulate(withQuality);

    int triangleCdtCt = trConstrGenerator.ntriangles();
   
    //assert(triangleCdtCt == 11);
    std::cout << " -- Constrained triangle count: " << triangleCdtCt << "\n";


   // 5. constrained triangulation with holes
   std::cout << "\nTEST: segment-constrainded triangluation (CDT) \n";

   std::vector<Delaunay::Point> constrDelaunayHoles;

   constrDelaunayHoles.push_back(Delaunay::Point(5, 1));
   constrDelaunayHoles.push_back(Delaunay::Point(5, 2));
   constrDelaunayHoles.push_back(Delaunay::Point(6, 2));
   constrDelaunayHoles.push_back(Delaunay::Point(6, 1));

   trConstrGenerator.setHolesConstraint(constrDelaunayHoles);
   trConstrGenerator.Triangulate(withQuality);

   triangleCdtCt = trConstrGenerator.ntriangles();

   std::cout << " -- Constrained triangle count with holes: " << triangleCdtCt << "\n";


    // 5.a -- unconstrained triangulation with holes

    std::vector<Delaunay::Point> zeroSegments;

    trConstrGenerator.setSegmentConstraint(zeroSegments);
    trConstrGenerator.Triangulate(withQuality);

    triangleCdtCt = trConstrGenerator.ntriangles();

    std::cout << " -- Unconstrained triangle count with holes: " << triangleCdtCt << "\n";


         
   // 6. ready!
   std::cout << "\nTEST: completed ---" << std::endl;
}

// --- eof ---
