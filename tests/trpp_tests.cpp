/*! \file trpp_tests.cpp
    \brief tests for the Triangle++ code
 */

#include "tpp_interface.hpp"

#include <vector>
#include <iostream>
#include <cassert>

// debug support
#define DEBUG_OUTPUT_STDOUT false 

using namespace tpp;

namespace {

    // impl. constants
#if DEBUG_OUTPUT_STDOUT
    tpp::DebugOutputLevel dbgOutput = tpp::Debug; // OR - tpp::Info
#else
    tpp::DebugOutputLevel dbgOutput = tpp::None;
#endif

    // impl. helpers
    void getTriangulationPoint(int keypointIdx, const Delaunay::Point& steinerPt, double& x, double& y, const std::vector<Delaunay::Point>& triPoints)
    {
#if DEBUG_OUTPUT_STDOUT
        std::cout << " --- keypointIdx= " << keypointIdx << "\n";
#endif
        if (keypointIdx == -1)
        {
            x = steinerPt[0]; // added Steiner point!
            y = steinerPt[1];
        }
        else
        {
            // point from original data
            x = triPoints[keypointIdx][0];
            y = triPoints[keypointIdx][1];
        }
    }
        
    void debugPrintTriangles(/*const*/ Delaunay& trGenerator, const std::vector<Delaunay::Point>& triPoints)
    {
        assert(trGenerator.hasTriangulation());

        if (dbgOutput == tpp::None)
        {
            return;
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

            getTriangulationPoint(keypointIdx1, sp1, x1, y1, triPoints);
            getTriangulationPoint(keypointIdx2, sp2, x2, y2, triPoints);
            getTriangulationPoint(keypointIdx3, sp3, x3, y3, triPoints);

            std::cout << " -- Triangle points: "
                << "{" << x1 << ", " << y1 << "}, "
                << "{" << x2 << ", " << y2 << "}, "
                << "{" << x3 << ", " << y3 << "}\n";
        }
    }

    bool checkConstraints(const Delaunay& trGenerator)
    {
        bool relaxedTest = true;

        if (!trGenerator.checkConstraintsOpt(relaxedTest))
        {
            std::cout << "TEST: constraints out of bounds, skip! \n";
            return false;
        }

        return true;
    };
}


int main()
{
    // prepare input
    std::vector<Delaunay::Point> delaunayInput;
    
    delaunayInput.push_back(Delaunay::Point(0,0));
    delaunayInput.push_back(Delaunay::Point(1,1));
    delaunayInput.push_back(Delaunay::Point(0,2));
    delaunayInput.push_back(Delaunay::Point(3,3));
    delaunayInput.push_back(Delaunay::Point(1.5, 2.125));

    // 1. standard triangulation
    std::cout << "TEST: standard triangulation \n";

    Delaunay trGenerator(delaunayInput);
    trGenerator.Triangulate(dbgOutput);

    debugPrintTriangles(trGenerator, delaunayInput);

    int triangleCt = trGenerator.ntriangles();
    int expected = 4;

    std::cout << " -- Standard triangle count: " << triangleCt << "\n";
    assert(triangleCt == expected);

    // 2. triangulate with quality constraints
    bool withQuality = true;

#if 1
    int nrOfConstraintTests = 4;

    for (int i = 0; i < nrOfConstraintTests; ++i)
    {
       if (i == 0)
       {
          // 2.1
          std::cout << "\nTEST: default constraints (min angle = 20°) \n";
          expected = 7;
          
          trGenerator.Triangulate(withQuality, dbgOutput);          
       }
       else if (i == 1)
       {
           // 2.2
           std::cout << "\nTEST: custom constraints (angle = 27.5°) \n";
           expected = 11;

           trGenerator.setMinAngle(27.5f);

           if (!checkConstraints(trGenerator))
           {
               continue;
           }

           trGenerator.Triangulate(withQuality, dbgOutput);
       }
       else if (i == 2)
       {
          // 2.3
          std::cout << "\nTEST: custom constraints (angle = 30.5°, area = 5.5) \n";
          expected = 17;
          
          trGenerator.setMinAngle(30.5f);
          trGenerator.setMaxArea(5.5f);

          if (!checkConstraints(trGenerator))
          {
             continue;
          }

          trGenerator.Triangulate(withQuality, dbgOutput);
       }
       else if (i == 3)
       {
          // 2.4
          std::cout << "\nTEST: custom constraints (angle = 44°) \n";
          
          // 44 deg results in an endless loop 
          //  --> triangles too tiny for the floating point precision! 
          trGenerator.setMinAngle(44.0f);
          trGenerator.setMaxArea(-1);

          if (!checkConstraints(trGenerator))
          {
             continue;
          }

          assert(false && "TEST: should never be reached!!!");

          trGenerator.Triangulate(withQuality, dbgOutput);
       }

       debugPrintTriangles(trGenerator, delaunayInput);

       triangleCt = trGenerator.ntriangles();

       std::cout << " -- Quality triangle count: " << triangleCt << "\n";
       assert(triangleCt == expected);
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

    int voronoiPoints = trGenerator.nvpoints();
    expected = 4;

    std::cout << " -- Voronoi points count: " << voronoiPoints << "\n";
    assert(voronoiPoints == expected);

    // OPEN TODO:::

    // iterate over Voronoi edges !!!!
    // use conforming Delaunay as base !!!!
   
    // OPEN TODO::: ...
#endif


#if 1
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
    trConstrGenerator.Triangulate(withQuality, dbgOutput);

    triangleCt = trConstrGenerator.ntriangles();
    expected = 11;
   
    std::cout << " -- Unconstrained triangle count: " << triangleCt << "\n";
    assert(triangleCt == expected);

    // prepare segments 
    //    - see "example constr segments.jpg" for visualisation!
    std::vector<Delaunay::Point> constrDelaunaySegments;

    constrDelaunaySegments.push_back(Delaunay::Point(0, 1));
    constrDelaunaySegments.push_back(Delaunay::Point(9, 0.75));

    // - CDT triangulation

    trConstrGenerator.setSegmentConstraint(constrDelaunaySegments);
    trConstrGenerator.useConvexHullWithSegments(true);

    trConstrGenerator.Triangulate(withQuality, dbgOutput);

    int triangleCdtCt = trConstrGenerator.ntriangles();
    expected = 36;

    std::cout << " -- Constrained triangle count: " << triangleCdtCt << "\n";
    assert(triangleCdtCt == expected);

    // - CDT triangulation w/o quality constr. 

    trConstrGenerator.Triangulate(dbgOutput);

    triangleCdtCt = trConstrGenerator.ntriangles();
    expected = 29;

    std::cout << " -- Constrained triangle count (quality=false): " << triangleCdtCt << "\n";
    assert(triangleCdtCt == expected);


   // 5. constrained triangulation with holes
   std::cout << "\nTEST: segment-constrainded triangluation (CDT) \n";

   std::vector<Delaunay::Point> constrDelaunayHoles;

   constrDelaunayHoles.push_back(Delaunay::Point(5, 1));
   constrDelaunayHoles.push_back(Delaunay::Point(5, 2));
   constrDelaunayHoles.push_back(Delaunay::Point(6, 2));
   constrDelaunayHoles.push_back(Delaunay::Point(6, 1));

   trConstrGenerator.setHolesConstraint(constrDelaunayHoles);
   trConstrGenerator.Triangulate(withQuality, dbgOutput);

   triangleCdtCt = trConstrGenerator.ntriangles();
   expected = 14;

   std::cout << " -- Constrained triangle count with holes: " << triangleCdtCt << "\n";
   assert(triangleCdtCt == expected);


    // 5.a -- unconstrained triangulation with holes
    std::vector<Delaunay::Point> zeroSegments;

    std::vector<Delaunay::Point> unconstrDelaunayHoles;
    unconstrDelaunayHoles.push_back(Delaunay::Point(0.25, 0.25));

    trConstrGenerator.setSegmentConstraint(zeroSegments);
    trConstrGenerator.setHolesConstraint(unconstrDelaunayHoles);

    trConstrGenerator.Triangulate(withQuality, dbgOutput);
    triangleCdtCt = trConstrGenerator.ntriangles();
    expected = 0; // all triangles infected as non edges required to be in triangualtion!

    std::cout << " -- Unconstrained triangle count with holes: " << triangleCdtCt << "\n";
    assert(triangleCdtCt == expected);
#endif


    // TEST::: new... ---> PLSG !!!

#if 1
    // 6. Planar Straight Line Graph (PSLG) triangulations
    std::cout << "\nTEST: Planar Straight Line Graph (PSLG) points-only triangluation \n";

    // prepare points: 
    //   - letter A, as in Triangle's documentation but simplified (https://www.cs.cmu.edu/~quake/triangle.defs.html#dt)
    std::vector<Delaunay::Point> pslgDelaunayInput;

    pslgDelaunayInput.push_back(Delaunay::Point(0, 0));
    pslgDelaunayInput.push_back(Delaunay::Point(1, 0));
    pslgDelaunayInput.push_back(Delaunay::Point(3, 0));
    pslgDelaunayInput.push_back(Delaunay::Point(4, 0));

    pslgDelaunayInput.push_back(Delaunay::Point(1.5, 1));
    pslgDelaunayInput.push_back(Delaunay::Point(2.5, 1));

    pslgDelaunayInput.push_back(Delaunay::Point(1.6, 1.5));
    pslgDelaunayInput.push_back(Delaunay::Point(2.4, 1.5));

    pslgDelaunayInput.push_back(Delaunay::Point(2, 2));
    
    pslgDelaunayInput.push_back(Delaunay::Point(3, 3));

    // - reference triangulation
    Delaunay trPlsgGenerator(pslgDelaunayInput);
    trPlsgGenerator.Triangulate(/*withQuality = false*/ dbgOutput); // ???? OPEN::: can be done?

    debugPrintTriangles(trPlsgGenerator, pslgDelaunayInput);

    triangleCt = trPlsgGenerator.ntriangles();
    expected = 12;

    std::cout << " -- Unconstrained triangle count: " << triangleCt << "\n";
    assert(triangleCt == expected);

 
    // prepare segments 
    //   - letter A, as in Triangle's documentation but simplified (https://www.cs.cmu.edu/~quake/triangle.defs.html#dt)
    std::vector<Delaunay::Point> pslgDelaunaySegments;

    // outer outline
    pslgDelaunaySegments.push_back(Delaunay::Point(1, 0));
    pslgDelaunaySegments.push_back(Delaunay::Point(0, 0));
           
    pslgDelaunaySegments.push_back(Delaunay::Point(0, 0));
    pslgDelaunaySegments.push_back(Delaunay::Point(3, 3));

    pslgDelaunaySegments.push_back(Delaunay::Point(3, 3));
    pslgDelaunaySegments.push_back(Delaunay::Point(4, 0));

    pslgDelaunaySegments.push_back(Delaunay::Point(4, 0));
    pslgDelaunaySegments.push_back(Delaunay::Point(3, 0));

    pslgDelaunaySegments.push_back(Delaunay::Point(3, 0));
    pslgDelaunaySegments.push_back(Delaunay::Point(2.5, 1));

    pslgDelaunaySegments.push_back(Delaunay::Point(2.5, 1));
    pslgDelaunaySegments.push_back(Delaunay::Point(1.5, 1));

    pslgDelaunaySegments.push_back(Delaunay::Point(1.5, 1));
    pslgDelaunaySegments.push_back(Delaunay::Point(1, 0));

    // inner outline
    pslgDelaunaySegments.push_back(Delaunay::Point(1.6, 1.5));
    pslgDelaunaySegments.push_back(Delaunay::Point(2, 2));

    pslgDelaunaySegments.push_back(Delaunay::Point(2, 2));
    pslgDelaunaySegments.push_back(Delaunay::Point(2.4, 1.5));

    pslgDelaunaySegments.push_back(Delaunay::Point(2.4, 1.5));
    pslgDelaunaySegments.push_back(Delaunay::Point(1.6, 1.5));
    

    // - CDT triangulation (quality)

    std::cout << "\nTEST: PSLG triangluation (quality=true) \n";

    bool segmentsOK = trPlsgGenerator.setSegmentConstraint(pslgDelaunaySegments);
    assert(segmentsOK);

    trPlsgGenerator.Triangulate(withQuality, dbgOutput);

    debugPrintTriangles(trPlsgGenerator, pslgDelaunayInput);

    int trianglePslgCt = trPlsgGenerator.ntriangles();
    expected = 50; 

    std::cout << " -- Constrained triangle count (quality=true): " << trianglePslgCt << "\n";
    assert(trianglePslgCt == expected);


    // OPEN TODO:: check --> concavities removed?


    // - CDT triangulation (standard)

    std::cout << "\nTEST: PSLG triangluation (quality=false) \n";

    trPlsgGenerator.Triangulate(dbgOutput);

    
    debugPrintTriangles(trPlsgGenerator, pslgDelaunayInput);

    trianglePslgCt = trPlsgGenerator.ntriangles();
    expected = 35;

    std::cout << " -- Constrained triangle count (quality=false): " << trianglePslgCt << "\n";
    assert(trianglePslgCt == expected);

#endif


   // 7. ready!
   std::cout << "\nTEST: tests completed ---" << std::endl;
}

// --- eof ---
