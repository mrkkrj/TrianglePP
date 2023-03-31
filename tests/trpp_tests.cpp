 /**
    @file  trpp_tests.cpp
    @brief test cases for the Triangle++ code
 */

#include "tpp_interface.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <vector>
#include <iostream>
#include <cassert>
#if __cplusplus >= 202002L
#include <numbers> // C++20!
#endif
#include <algorithm>

// debug support
#define DEBUG_OUTPUT_STDOUT false 
//#define DEBUG_OUTPUT_STDOUT true 

using namespace tpp;

namespace {

#if DEBUG_OUTPUT_STDOUT
    //tpp::DebugOutputLevel dbgOutput = tpp::Debug; // OR - tpp::Info
    tpp::DebugOutputLevel dbgOutput = tpp::Info; 
#else
    tpp::DebugOutputLevel dbgOutput = tpp::None;
#endif

    // impl. helpers

    void getTriangulationPoint(int keypointIdx, const Delaunay::Point& steinerPt, 
                               double& x, double& y, const std::vector<Delaunay::Point>& triPoints)
    {
        if (dbgOutput != tpp::None)
           std::cout << " --- keypointIdx= " << keypointIdx << "\n";

        if (keypointIdx == -1)
        {
            x = steinerPt[0]; // added Steiner point!
            y = steinerPt[1];
        }
        else
        {
            // point from original data
            assert(keypointIdx >= 0);

            x = triPoints[static_cast<unsigned>(keypointIdx)][0];
            y = triPoints[static_cast<unsigned>(keypointIdx)][1];
        }
    }
        
    void debugPrintTriangles(/*const*/ Delaunay& trGenerator, const std::vector<Delaunay::Point>& triPoints)
    {
        REQUIRE(trGenerator.hasTriangulation());

        if (dbgOutput == tpp::None)
        {
            return;
        }

        // iterate over triangles
        for (FaceIterator fit = trGenerator.fbegin(); fit != trGenerator.fend(); ++fit)
        {
            Delaunay::Point sp1;
            Delaunay::Point sp2;
            Delaunay::Point sp3;

            int keypointIdx1 = fit.Org(&sp1);
            int keypointIdx2 = fit.Dest(&sp2);
            int keypointIdx3 = fit.Apex(&sp3);

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

    void debugPrintVoronoiPoints(/*const*/ Delaunay& trGenerator)
    {
        REQUIRE(trGenerator.hasTriangulation());

        if (dbgOutput == tpp::None)
        {
            return;
        }

       // iterate over Voronoi points
       for (tpp::VoronoiVertexIterator vit = trGenerator.vvbegin(); vit != trGenerator.vvend(); ++vit)
       {
          Delaunay::Point vp = *vit;
          double x1 = vp[0];
          double y1 = vp[1];

          std::cout << " -- Voronoi point: " << "{" << x1 << "," << y1 << "}\n";
       }

       std::cout << " -- Voronoi points count: " << trGenerator.voronoiPointCount() << "\n";
    }

    void checkTriangleCount(/*const*/ Delaunay& trGenerator, const std::vector<Delaunay::Point>& delaunayInput, 
                            int expected, const char* descr = nullptr)
    {
       debugPrintTriangles(trGenerator, delaunayInput);

       int triangleCt = trGenerator.triangleCount();

       if (dbgOutput != tpp::None)
          std::cout << " - " << (descr ? descr : "") << " triangle count: " << triangleCt << "\n\n";

       REQUIRE(triangleCt == expected);
    }

    bool checkConstraints(const Delaunay& trGenerator)
    {
        bool relaxedTest = true;

        if (!trGenerator.checkConstraintsOpt(relaxedTest))
        {
            if (dbgOutput != tpp::None)
               std::cout << " -- constraints out of bounds!!!\n";

            return false;
        }

        return true;
    };


    // test data:
    //  - letter A, as in Triangle's documentation but simplified (https://www.cs.cmu.edu/~quake/triangle.defs.html#dt)

    void preparePLSGTestData(std::vector<Delaunay::Point>& pslgPoints, std::vector<Delaunay::Point>& pslgSegments)
    {
        // prepare points
        pslgPoints.push_back(Delaunay::Point(0, 0));
        pslgPoints.push_back(Delaunay::Point(1, 0));
        pslgPoints.push_back(Delaunay::Point(3, 0));
        pslgPoints.push_back(Delaunay::Point(4, 0));
        pslgPoints.push_back(Delaunay::Point(1.5, 1));
        pslgPoints.push_back(Delaunay::Point(2.5, 1));
        pslgPoints.push_back(Delaunay::Point(1.6, 1.5));
        pslgPoints.push_back(Delaunay::Point(2.4, 1.5));

        pslgPoints.push_back(Delaunay::Point(2, 2));
        pslgPoints.push_back(Delaunay::Point(2, 3));

        // prepare segments
        //  - outer outline
        pslgSegments.push_back(Delaunay::Point(1, 0));
        pslgSegments.push_back(Delaunay::Point(0, 0));
        pslgSegments.push_back(Delaunay::Point(0, 0));
        pslgSegments.push_back(Delaunay::Point(2, 3));
        pslgSegments.push_back(Delaunay::Point(2, 3));
        pslgSegments.push_back(Delaunay::Point(4, 0));
        pslgSegments.push_back(Delaunay::Point(4, 0));
        pslgSegments.push_back(Delaunay::Point(3, 0));
        pslgSegments.push_back(Delaunay::Point(3, 0));
        pslgSegments.push_back(Delaunay::Point(2.5, 1));
        pslgSegments.push_back(Delaunay::Point(2.5, 1));
        pslgSegments.push_back(Delaunay::Point(1.5, 1));
        pslgSegments.push_back(Delaunay::Point(1.5, 1));
        pslgSegments.push_back(Delaunay::Point(1, 0));

        // - inner outline
        pslgSegments.push_back(Delaunay::Point(1.6, 1.5));
        pslgSegments.push_back(Delaunay::Point(2, 2));
        pslgSegments.push_back(Delaunay::Point(2, 2));
        pslgSegments.push_back(Delaunay::Point(2.4, 1.5));
        pslgSegments.push_back(Delaunay::Point(2.4, 1.5));
        pslgSegments.push_back(Delaunay::Point(1.6, 1.5));
    }
}


// test cases

TEST_CASE("standard and quality triangulations", "[trpp]")
{
    // prepare input
    std::vector<Delaunay::Point> delaunayInput;
    
    delaunayInput.push_back(Delaunay::Point(0,0));
    delaunayInput.push_back(Delaunay::Point(1,1));
    delaunayInput.push_back(Delaunay::Point(0,2));
    delaunayInput.push_back(Delaunay::Point(3,3));
    delaunayInput.push_back(Delaunay::Point(1.5, 2.125));

    // 1. standard triangulation

    Delaunay trGenerator(delaunayInput);
    int triangleCt = 0;
    int expected = 0;

    SECTION("TEST 1: standard triangulation") 
    {
       trGenerator.Triangulate(dbgOutput);

       expected = 4;
       checkTriangleCount(trGenerator, delaunayInput, expected, "Standard");
    }

    // 2. triangulate with quality constraints

    bool withQuality = true;

    SECTION("TEST 2.1: default quality (min angle = 20°)")
    {
       trGenerator.Triangulate(withQuality, dbgOutput);    

       expected = 7;
       checkTriangleCount(trGenerator, delaunayInput, expected, "Quality");
    }

    SECTION("TEST 2.2: custom quality (angle = 27.5°)")
    {
       trGenerator.setMinAngle(27.5f);
       REQUIRE(checkConstraints(trGenerator) == true);

       trGenerator.Triangulate(withQuality, dbgOutput);

       expected = 11;
       checkTriangleCount(trGenerator, delaunayInput, expected);
    }

    SECTION("TEST 2.3: custom quality (angle = 30.5°, area = 5.5)")
    {
       trGenerator.setMinAngle(30.5f);
       trGenerator.setMaxArea(5.5f);
       REQUIRE(checkConstraints(trGenerator) == true);

       trGenerator.Triangulate(withQuality, dbgOutput);

       expected = 17;
       checkTriangleCount(trGenerator, delaunayInput, expected);
    }

    SECTION("TEST 2.4: custom quality (angle = 44°)")
    {
       // 44 deg results in an endless loop 
       //  --> triangles too tiny for the floating point precision! 
       trGenerator.setMinAngle(44.0f);
       trGenerator.setMaxArea(-1);

       REQUIRE(checkConstraints(trGenerator) == false);               
    }
}


TEST_CASE("Voronoi tesselation", "[trpp]")
{
    // prepare input
    std::vector<Delaunay::Point> delaunayInput;

    delaunayInput.push_back(Delaunay::Point(0,0));
    delaunayInput.push_back(Delaunay::Point(1,1));
    delaunayInput.push_back(Delaunay::Point(0,2));
    delaunayInput.push_back(Delaunay::Point(3,3));
    delaunayInput.push_back(Delaunay::Point(1.5, 2.125));

    Delaunay trGenerator(delaunayInput);

    // 3. Voronoi diagram

    SECTION("TEST 3: Voronoi tesselation")
    {
       trGenerator.Tesselate();
       debugPrintVoronoiPoints(trGenerator);

       int voronoiPoints = trGenerator.voronoiPointCount();
       int expected = 4;

       REQUIRE(voronoiPoints == expected);
    }
}


TEST_CASE("segment-constrainded triangluation (CDT)", "[trpp]")
{
    // prepare input 
    //  - see "example constr segments.jpg" for visualisation!
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

    int expected = 0;
    int referenceCt = 0;
    int referenceQualityCt = 0;
    bool withQuality = true;

    Delaunay trConstrGenerator(constrDelaunayInput);

    SECTION("TEST 4.0: reference triangulation (without quality constr.)")
    {
        trConstrGenerator.Triangulate(!withQuality, dbgOutput);
        referenceCt = trConstrGenerator.triangleCount();

        expected = 11;
        checkTriangleCount(trConstrGenerator, constrDelaunayInput, expected, "Unconstrained (quality=false)");

        REQUIRE(referenceCt == expected);
    }

    SECTION("TEST 4.0: reference triangulation with quality constr.")
    {
        trConstrGenerator.Triangulate(withQuality, dbgOutput);
        referenceQualityCt = trConstrGenerator.triangleCount();

        expected = 11; // checked with GUI
        checkTriangleCount(trConstrGenerator, constrDelaunayInput, expected, "Unconstrained (quality=true)");
    }

    // prepare segments 
    //  - see "example constr segments.jpg" for visualisation!
    std::vector<Delaunay::Point> constrDelaunaySegments;

    constrDelaunaySegments.push_back(Delaunay::Point(0, 1));
    constrDelaunaySegments.push_back(Delaunay::Point(9, 0.75));

    // 4. segment-constrained triangulation

    trConstrGenerator.setSegmentConstraint(constrDelaunaySegments);
    trConstrGenerator.useConvexHullWithSegments(true); // don't remove concavities!

    SECTION("TEST 4.1: CDT triangulation (without quality constr.)")
    {
        trConstrGenerator.Triangulate(dbgOutput);

        expected = 11; // count not changed, see "example constr segments.jpg" for visualisation!
        checkTriangleCount(trConstrGenerator, constrDelaunayInput, expected, "Constrained (quality=false)");

        // OPEN TODO:::
        //  -- but different triangles!!!! 
        //  CHECK( .... )
    }

    SECTION("TEST 4.2: CDT triangulation using endpoint indexes")
    {        
        std::vector<int> segmentsEndpointIdx;
        segmentsEndpointIdx.push_back(0);
        segmentsEndpointIdx.push_back(9);

        trConstrGenerator.setSegmentConstraint(std::vector<Delaunay::Point>());
        trConstrGenerator.setSegmentConstraint(segmentsEndpointIdx);

        trConstrGenerator.Triangulate(dbgOutput);

        expected = 11; // count not changed, see "example constr segments.jpg" for visualisation!
        checkTriangleCount(trConstrGenerator, constrDelaunayInput, expected, "Constrained (endpoint indexes)");


        // OPEN TODO:: specify more segments by IDX ...

    }

    SECTION("TEST 4.3: CDT triangulation with quality constr.")
    {
        trConstrGenerator.Triangulate(withQuality, dbgOutput);

        expected = 29; // checked with GUI
        checkTriangleCount(trConstrGenerator, constrDelaunayInput, expected, "Constrained (quality=true)");
    }

    // 5. triangulation with holes

    SECTION("TEST 5.1: holes + segment-constrainded triangluation (CDT)") 
    { 
       std::vector<Delaunay::Point> constrDelaunayHoles;

       constrDelaunayHoles.push_back(Delaunay::Point(5, 1));
       constrDelaunayHoles.push_back(Delaunay::Point(5, 2));
       constrDelaunayHoles.push_back(Delaunay::Point(6, 2));
       constrDelaunayHoles.push_back(Delaunay::Point(6, 1));

       trConstrGenerator.setHolesConstraint(constrDelaunayHoles);
       trConstrGenerator.Triangulate(withQuality, dbgOutput);
       
       expected = 11; // checked with GUI
       checkTriangleCount(trConstrGenerator, constrDelaunayInput, expected, "Constrained + holes (quality=true)");

       trConstrGenerator.Triangulate(!withQuality, dbgOutput);

       expected = 4; // checked with GUI
       checkTriangleCount(trConstrGenerator, constrDelaunayInput, expected, "Constrained + holes (quality=true)");
    }

    SECTION("TEST 5.1: holes + unconstrainded triangluation (CDT)") 
    { 
       std::vector<Delaunay::Point> zeroSegments;
       std::vector<Delaunay::Point> unconstrDelaunayHoles;

       unconstrDelaunayHoles.push_back(Delaunay::Point(0.25, 0.25));

       trConstrGenerator.setSegmentConstraint(zeroSegments);
       trConstrGenerator.setHolesConstraint(unconstrDelaunayHoles);

       trConstrGenerator.Triangulate(withQuality, dbgOutput);
   
       expected = 0; // all triangles infected as non edges required to be in triangualtion!
       checkTriangleCount(trConstrGenerator, constrDelaunayInput, expected, "Unconstrained + holes");
    }
}


TEST_CASE("Planar Straight Line Graph (PSLG) triangulation", "[trpp]")
{
    // prepare points & segments of a PSLG (simplified letter "A")
    std::vector<Delaunay::Point> pslgDelaunayInput;
    std::vector<Delaunay::Point> pslgDelaunaySegments;

    preparePLSGTestData(pslgDelaunayInput, pslgDelaunaySegments);

    // 6. Planar Straight Line Graph (PSLG) triangulations

    int expected = 0;
    bool withQuality = true;

    Delaunay trPlsgGenerator(pslgDelaunayInput);

    SECTION("TEST 6.1: Planar Straight Line Graph (PSLG) points-only triangluation") 
    {
       trPlsgGenerator.Triangulate(!withQuality, dbgOutput);

       expected = 13;
       checkTriangleCount(trPlsgGenerator, pslgDelaunayInput, expected, "Unconstrained (quality=false)");

       trPlsgGenerator.Triangulate(withQuality, dbgOutput);

       expected = 15;
       checkTriangleCount(trPlsgGenerator, pslgDelaunayInput, expected, "Unconstrained (quality=true)");
    }

    SECTION("TEST 6.2: PSLG triangluation (quality=true)") 
    {
       bool segmentsOK = trPlsgGenerator.setSegmentConstraint(pslgDelaunaySegments);
       REQUIRE(segmentsOK);
       trPlsgGenerator.Triangulate(withQuality, dbgOutput);

       expected = 13; // checked with GUI
       checkTriangleCount(trPlsgGenerator, pslgDelaunayInput, expected, "Constrained (quality=true)");

       // concavities removed?
       trPlsgGenerator.useConvexHullWithSegments(true);
       trPlsgGenerator.Triangulate(withQuality, dbgOutput);

       expected = 15; // checked with GUI
       checkTriangleCount(trPlsgGenerator, pslgDelaunayInput, expected, "Constrained + convex hull (quality=true)");
    }

    SECTION("TEST 6.3: PSLG triangluation (quality=false)") 
    {
       bool segmentsOK = trPlsgGenerator.setSegmentConstraint(pslgDelaunaySegments);
       REQUIRE(segmentsOK);
       trPlsgGenerator.useConvexHullWithSegments(false);

       trPlsgGenerator.Triangulate(!withQuality, dbgOutput);

       expected = 11; // checked with GUI
       checkTriangleCount(trPlsgGenerator, pslgDelaunayInput, expected, "Constrained (quality=false)");

       // concavities removed?
       trPlsgGenerator.useConvexHullWithSegments(true);
       trPlsgGenerator.Triangulate(!withQuality, dbgOutput);

       expected = 13; // checked with GUI
       checkTriangleCount(trPlsgGenerator, pslgDelaunayInput, expected, "Constrained + convex hull (quality=false)");
    }
}


TEST_CASE("Reading files", "[trpp]")
{
    Delaunay trReader;
    bool ioStatus = false;

    // 7. reading files

    SECTION("TEST 7.1: reading a .node file")
    {
        std::vector<Delaunay::Point> points;

        ioStatus = trReader.readPoints("../tppDataFiles/spiral.node", points);

        REQUIRE(ioStatus == true);
        REQUIRE(points.size() == 15); // look inside the file

        ioStatus = trReader.readPoints("../tppDataFiles/dots.node", points);

        REQUIRE(ioStatus == true);
        REQUIRE(points.size() == 100); // look inside the file
    }

    SECTION("TEST 7.3: reading .poly file")
    {
        std::vector<Delaunay::Point> points;
        std::vector<int> segments;
        std::vector<Delaunay::Point> holes;

        ioStatus = trReader.readSegments("../tppDataFiles/face.poly", points, segments, holes);

        REQUIRE(ioStatus == true);
        REQUIRE(points.size() == 26);       // look inside the file
        REQUIRE(segments.size()/2  == 22);  // look inside the file
        REQUIRE(holes.size() == 3);         // look inside the file

        ioStatus = trReader.readSegments("../tppDataFiles/box.poly", points, segments, holes);

        REQUIRE(ioStatus == true);
        REQUIRE(points.size() == 8);        // look inside the file
        REQUIRE(segments.size() / 2 == 5);  // look inside the file
        REQUIRE(holes.size() == 1);         // look inside the file

        ioStatus = trReader.readSegments("../tppDataFiles/la.poly", points, segments, holes);

        REQUIRE(ioStatus == true);
        REQUIRE(points.size() == 141);        // look inside the file
        REQUIRE(segments.size() / 2 == 148);  // look inside the file
        REQUIRE(holes.size() == 0);           // look inside the file
    }
}


TEST_CASE("Writing files", "[trpp]")
{
    // 8. writing files

    std::vector<Delaunay::Point> pslgDelaunayInput;
    std::vector<Delaunay::Point> pslgDelaunaySegments;

    preparePLSGTestData(pslgDelaunayInput, pslgDelaunaySegments);

    Delaunay trWriter(pslgDelaunayInput);
    Delaunay trReader;
    bool ioStatus = false;

    SECTION("TEST 8.1: writing a .node file")
    {
        ioStatus = trWriter.savePoints("./test.node");
        REQUIRE(ioStatus == true);

        // read it back
        std::vector<Delaunay::Point> points;
        ioStatus = trReader.readPoints("./test.node", points);

        REQUIRE(ioStatus == true);
        REQUIRE(points.size() == pslgDelaunayInput.size());
    }

    SECTION("TEST 8.2: writing a .poly file containing segments")
    {
        bool segmentsOK = trWriter.setSegmentConstraint(pslgDelaunaySegments);
        REQUIRE(segmentsOK);

        ioStatus = trWriter.saveSegments("./test.poly");
        REQUIRE(ioStatus == true);

        // read it back
        std::vector<Delaunay::Point> points;
        std::vector<int> segments;
        std::vector<Delaunay::Point> holes;

        ioStatus = trReader.readSegments("./test.poly", points, segments, holes);

        REQUIRE(ioStatus == true);
        REQUIRE(points.size() == pslgDelaunayInput.size());
        REQUIRE(segments.size() == pslgDelaunaySegments.size()); 
        REQUIRE(holes.size() == 0); // no holes!
    }

    SECTION("TEST 8.2: writing a .poly file containig segments & holes")
    {
        std::vector<Delaunay::Point> pslgHoles;
        pslgHoles.push_back(Delaunay::Point(1, 1));
        pslgHoles.push_back(Delaunay::Point(1, 2));

        bool holesOK = trWriter.setHolesConstraint(pslgHoles);
        REQUIRE(holesOK);

        bool segmentsOK = trWriter.setSegmentConstraint(pslgDelaunaySegments);
        REQUIRE(segmentsOK);

        // OPEN TODO::: not yet working on Windows!!!
        //  -- only 3 segments saved!!!!!
        ioStatus = trWriter.saveSegments("./test.poly"); 
        REQUIRE(ioStatus == true);

        // read it back
        std::vector<Delaunay::Point> points;
        std::vector<int> segments;
        std::vector<Delaunay::Point> holes;

        ioStatus = trReader.readSegments("./test.poly", points, segments, holes);

        REQUIRE(ioStatus == true);
        REQUIRE(points.size() == pslgDelaunayInput.size());
        REQUIRE(segments.size() == pslgDelaunaySegments.size());  
        REQUIRE(holes.size() == pslgHoles.size());
    }
}


TEST_CASE("Segment-constrained triangulation with duplicates", "[trpp]")
{
    // 9. Duplicate points

    int expected = 0;
    bool withQuality = true;

    SECTION("TEST 9.1: PSLG triangluation with duplicate points used in segments")
    {
        // Testdata: as specified in ../tppDataFiles/box-continuous.poly
        std::vector<Delaunay::Point> pslgDelaunayInput;
        std::vector<int> pslgSegmentEndpointIdx;
                
        pslgDelaunayInput = {
            { 0.0000,   0.0000 },
            // start of inner shape
            { 0.5000,   0.0000 },
            { 0.5000,   0.2500 },
            { 0.2500,   0.2500 },
            { 0.2500,   0.7500 },
            { 0.7500,   0.7500 },
            { 0.7500,   0.2500 },
            { 0.5000,   0.2500 },
            { 0.5000,   0.0000 },
            // end of inner shape
            { 1.0000,   0.0000 },
            { 1.0000,   1.0000 },
            { 0.0000,   1.0000 },
            { 0.0000,   0.0000 }
        };

        pslgSegmentEndpointIdx = {
               0,   1,      1,   2,
               2,   3,      3,   4,
               4,   5,      5,   6,
               6,   7,      7,   8,
               8,   9,      9,  10,
              10,  11,     11,  12,
              12,  0
        };

       Delaunay trPlsgGenerator(pslgDelaunayInput);

       bool segmentsOK = trPlsgGenerator.setSegmentConstraint(pslgSegmentEndpointIdx, dbgOutput);
       REQUIRE(segmentsOK);

       trPlsgGenerator.Triangulate(dbgOutput);

       expected = 13;
       checkTriangleCount(trPlsgGenerator, pslgDelaunayInput, expected, "PSLG duplicate points");
    }

    // TODO:::
    //  SECTION("TEST 9.2: PSLG triangluation with duplicate points NOT used in segments")
    
    //  SECTION("TEST 9.3: PSLG triangluation with duplicate segments")
    
    //  SECTION("TEST 9.4: PSLG triangluation with duplicate holes")
    
    //  SECTION("TEST 9.5: PSLG triangluation with colinear segments")

    //SECTION("TEST 9.1: PSLG triangluation with duplicate points used in segments --> segments BY point coordinates")

 
    // 10. Duplicate segments

    SECTION("TEST 10.1: PSLG triangluation with duplicate points and duplicate segments")
    {
        // Testdata: as specified in ../tppDataFiles/hex-overlap.poly
        std::vector<Delaunay::Point> pslgDelaunayInput1;
        std::vector<int> pslgSegmentEndpointIdx1;

        pslgDelaunayInput1 = {
            /*1*/ { 0.0000,     0.0000 },   
            /*2*/ {  1.0000,    0.0000  }, 
            /*3*/ {  1.0000,    1.0000  }, 
            /*4*/ {  0.0000,    1.0000  }, 

            // Points around hexagonal hole 1.
            /*5*/ { 0.1500,    0.7500  },
            /*6*/ { 0.2003,    0.6634  },
            /*7*/ { 0.3002,    0.6640  },
            /*8*/ { 0.3500,    0.7500  },
            /*9*/ { 0.3002,    0.8360  },
            /*10*/ { 0.2003,    0.8366  },

            // Points around hexagonal hole 2.
            /*11*/ { 0.5000,    0.4000  },
            /*12*/ { 0.5503,    0.3134  },
            /*13*/ { 0.6502,    0.3140  },
            /*14*/ { 0.7000,    0.4000  },
            /*15*/ { 0.6502,    0.4860  },
            /*16*/ { 0.5503,    0.4866  },

            // copy of Points around hexagonal hole 2.
            /*17*/ { 0.5000,    0.4000 },
            /*18*/ { 0.5503,    0.3134  },
            /*19*/ { 0.6502,    0.3140  },
            /*20*/ { 0.7000,    0.4000  },
            /*21*/ { 0.6502,    0.4860  },
            /*22*/ { 0.5503,    0.4866  }
        };

        pslgSegmentEndpointIdx1 = {
            // Segment 1
            1,   2,            2,   3,
            3,   4,            4,   1,

            // Segment 2 around the first hexagonal hole.
            5,   6,            6,   7,
            7,   8,            8,   9,
            9,  10,            10,   5,
            
            // Segment 3 around the second hexagonal hole.
            11,  12,            12,  13,
            13,  14,            14,  15,
            15,  16,            16,  11,
            
            // copy of segment 3.
            17,  18,            18,  19,
            19,  20,            20,  21,
            21,  22,            22,  17
        };

        for (auto& index : pslgSegmentEndpointIdx1)
        {
            index -= 1; // zero based indexing!
        }

        std::vector<Delaunay::Point> constrDelaunayHoles =  {
            // Define a hole by giving the coordinates of one point inside it.
            { 0.2500,  0.7500 },
            { 0.6000,  0.4000 },
            { 0.6000,  0.4000 }
        };

        Delaunay trPlsgGenerator1(pslgDelaunayInput1);

        bool segmentsOK = trPlsgGenerator1.setSegmentConstraint(pslgSegmentEndpointIdx1, dbgOutput);
        REQUIRE(segmentsOK);

        bool holesOK = trPlsgGenerator1.setHolesConstraint(constrDelaunayHoles);
        REQUIRE(holesOK);

        trPlsgGenerator1.Triangulate(dbgOutput);

        expected = 18;
        checkTriangleCount(trPlsgGenerator1, pslgDelaunayInput1, expected, "PSLG duplicate points & segments");
    }

}
 

TEST_CASE("Usage of iterators", "[trpp]")
{
   //  ---- code by Ivan Notaros ---->
   class PointHasher {
   public:
      size_t operator() (const Delaunay::Point& key) const
      {
         std::size_t h1 = std::hash<double>{}(key[0]);
         std::size_t h2 = std::hash<double>{}(key[1]);
         return h1 ^ (h2 << 1);
      }
   };

   class PointKeyeq {
   public:
      bool operator() (const Delaunay::Point& p0, const Delaunay::Point& p1) const {
         return 
            p0[0] == p1[0] && p0[1] == p1[1];
      }
   };
   
   struct Vertex {
      float x, y, z;
      float colr, colg, colb;

      Vertex(float xp, float yp, float zp) : x(xp), y(yp), z(zp), colr(0.0f), colg(0.0f), colb(0.0f) {}
   };

   const auto ToVert = [](const Delaunay::Point& point) {
      Vertex v{ static_cast<float>(point[0]), static_cast<float>(point[1]), 0.f };
      return v;
   };

   struct Mesh {
      std::vector<int> indices;
      std::vector<Vertex> vertices;
   } 
   mesh;

   struct GenParams {
      float radius;
      int circleSegments;
      float minAngle, maxArea;
   }
   genParams;

   std::vector<Delaunay::Point> inputPoints;
   bool mergeVertices;

   const auto Regenerate = [&]() {
      inputPoints.clear();
      mesh.indices.clear();
      mesh.vertices.clear();

      // Make a circle of points
      {
         const float radX = genParams.radius;
         const float radY = genParams.radius;
         const float centerX = 0.f;
         const float centerY = 0.f;
         const int interpolations = genParams.circleSegments;

#if __cplusplus >= 202002L
         // C++20!
         float step = (2.f * std::numbers::pi_v<float>) / interpolations;
#else
         float step = (2.f * 3.141592653589793f) / interpolations;
#endif
         for (int i = 0; i < interpolations; i++)
         {
            float theta = i * step;

            auto p = tpp::Delaunay::Point(
               centerX + (cos(theta) * radX),
               centerY + (sin(theta) * radY)
            );

            inputPoints.push_back(p);
         }
      }

      tpp::Delaunay gen(inputPoints);

      gen.setMinAngle(genParams.minAngle);
      gen.setMaxArea(genParams.maxArea);
      gen.TriangulateConf(true);

      using Point = tpp::Delaunay::Point;

      // Obtain points and triangles
      std::unordered_map<Point, int, PointHasher, PointKeyeq> vertMap; // For merging
      vertMap.reserve(1024);

      const bool _merge = mergeVertices;

      int index = 0;
      for (tpp::FaceIterator it = gen.fbegin(); it != gen.fend(); ++it)
      {
         Point p0; it.Org(&p0); // These return index of a point in the input list (useless)!!!
         Point p1; it.Dest(&p1);
         Point p2; it.Apex(&p2);

         if (_merge) // with vertex merging
         {
            const auto AddVert = [&](const Point& _p) {
#if __cplusplus >= 202002L
               // C++20!
               if (vertMap.contains(_p)) // use existing vertex
#else
               if (vertMap.find(_p) != vertMap.end()) // use existing vertex
#endif
               {
                  mesh.indices.push_back(vertMap[_p]);
               }
               else // create a new vertex
               {
                  vertMap[_p] = index;
                  mesh.indices.push_back(index);
                  index++;

                  mesh.vertices.push_back(ToVert(_p));
               }
            };

            AddVert(p0);
            AddVert(p1);
            AddVert(p2);

         }
         else // without vertex merging
         {
            mesh.indices.push_back(index++);
            mesh.indices.push_back(index++);
            mesh.indices.push_back(index++);

            mesh.vertices.push_back(ToVert(p0));
            mesh.vertices.push_back(ToVert(p1));
            mesh.vertices.push_back(ToVert(p2));
         }
      }
   };
   //  ---- code by Ivan Notaros (end) ----> 

   // 1. run a reference test
   float radius = 10;
   int circleSegments = 100;
   float minAngle = 20;
   float maxArea = 20;

   genParams = { radius, circleSegments , minAngle, maxArea };
   mergeVertices = true;

   Regenerate();   

   REQUIRE(!inputPoints.empty());
   REQUIRE(!mesh.indices.empty());
   REQUIRE(!mesh.vertices.empty());

   // 2. run test sections
   using Point = tpp::Delaunay::Point;

   tpp::Delaunay gen(inputPoints);
   gen.enableMeshIndexGeneration();

   gen.setMinAngle(genParams.minAngle);
   gen.setMaxArea(genParams.maxArea);
   gen.TriangulateConf(true);

   Point p0, p1, p2;
   int meshIdx0 = -1, meshIdx1 = -1, meshIdx2 = -1;

   auto insertMeshPoint = [&](Mesh& mesh, int& vertexCount, const Point& pt, int vertexIdx)
   {
      if (vertexIdx > vertexCount)
      {
         // new point!
         mesh.vertices.push_back(ToVert(pt));
         assert(mesh.vertices.size() == ((std::size_t)vertexIdx + 1)); // correct indexing?

         assert(vertexIdx == vertexCount + 1); // vertex numbered as expected? 
         vertexCount = vertexIdx;
      }

      mesh.indices.push_back(vertexIdx);
   };

   SECTION("TEST 11.1: Vertex iterator usage")
   {
      Mesh mesh_Vi;

      for (tpp::VertexIterator it = gen.vbegin(); it != gen.vend(); ++it)
      {
         auto vertexId = it.vertexId();
         auto pt = *it;

         mesh_Vi.indices.push_back(vertexId);
         mesh_Vi.vertices.push_back(ToVert(pt));
      }

      std::cout << " ### vertices =" << mesh.vertices.size() << std::endl;
      std::cout << " ### vertices_Viter =" << mesh_Vi.vertices.size() << std::endl;

      std::cout << " ### indexes =" << mesh.indices.size() << std::endl;
      std::cout << " ### indexes_Viter =" << mesh_Vi.indices.size() << std::endl;

      // checks
      REQUIRE(mesh.indices.size() != mesh_Vi.indices.size()); // not equal!
      REQUIRE(mesh.vertices.size() == mesh_Vi.vertices.size());
   }

   SECTION("TEST 11.2: Face iterator with mesh indexing")
   {
      Mesh mesh_meFi;
      int count = -1;

      auto insertMeshPt = [&](const Point& pt, int vertexIdx) {
         insertMeshPoint(mesh_meFi, count, pt, vertexIdx);
      };

      for (tpp::FaceIterator it = gen.fbegin(); it != gen.fend(); ++it)
      {
         it.Org(p0, meshIdx0);
         insertMeshPt(p0, meshIdx0);

         it.Dest(p1, meshIdx1);
         insertMeshPt(p1, meshIdx1);

         it.Apex(p2, meshIdx2);
         insertMeshPt(p2, meshIdx2);

         //std::cout << " ### idx0=" << idx0 << " idx1=" << idx1 << " idx2=" << idx2 << std::endl;   
         REQUIRE(meshIdx0 >= 0); REQUIRE(meshIdx1 >= 0); REQUIRE(meshIdx2 >= 0);
      }

      std::cout << " ### vertices =" << mesh.vertices.size() << std::endl;
      std::cout << " ### vertices_MeshIter =" << mesh_meFi.vertices.size() << std::endl;

      std::cout << " ### indexes =" << mesh.indices.size() << std::endl;
      std::cout << " ### indexes_MeshIter =" << mesh_meFi.indices.size() << std::endl;

      // checks
      REQUIRE(mesh.indices.size() == mesh_meFi.indices.size());
      REQUIRE(mesh.vertices.size() == mesh_meFi.vertices.size());
   }

   SECTION("TEST 11.3: Delaunay::faces() with a foreach loop")
   {
      Mesh mesh_meFi;
      int count = -1;

      auto insertMeshPt = [&](const Point& pt, int vertexIdx) {
         insertMeshPoint(mesh_meFi, count, pt, vertexIdx);
      };

      for (const auto& f : gen.faces())
      {
         f.Org(p0, meshIdx0);
         insertMeshPt(p0, meshIdx0);

         f.Dest(p1, meshIdx1);
         insertMeshPt(p1, meshIdx1);

         f.Apex(p2, meshIdx2);
         insertMeshPt(p2, meshIdx2);

         //std::cout << " ### idx0=" << idx0 << " idx1=" << idx1 << " idx2=" << idx2 << std::endl;   
         REQUIRE(meshIdx0 >= 0); REQUIRE(meshIdx1 >= 0); REQUIRE(meshIdx2 >= 0);
      }

      REQUIRE(mesh.indices.size() == mesh_meFi.indices.size());
      REQUIRE(mesh.vertices.size() == mesh_meFi.vertices.size());
   }

   SECTION("TEST 11.4: Face iterator, but mesh indexing not enabled")
   {
      tpp::Delaunay genNoMeshIdx(inputPoints);

      genNoMeshIdx.setMinAngle(genParams.minAngle);
      genNoMeshIdx.setMaxArea(genParams.maxArea);
      genNoMeshIdx.TriangulateConf(true);

      auto iter = genNoMeshIdx.fbegin();

      // disable assertions for this line...
      bool tmp = tpp::g_disableAsserts;
      tpp::g_disableAsserts = true;

      // should throw (as in Release mode):
      REQUIRE_THROWS_MATCHES(iter.Org(p0, meshIdx0),
         std::runtime_error, Catch::Matchers::Message("Mesh indexing not enabled"));

      tpp::g_disableAsserts = tmp;
   }

   SECTION("TEST 11.4: Face iterator - triangle's area")
   {
      // 4 points forming a 2x2 square
      std::vector<Delaunay::Point> in;

      in.push_back(Delaunay::Point(0, 0));
      in.push_back(Delaunay::Point(0, 2));
      in.push_back(Delaunay::Point(2, 0));
      in.push_back(Delaunay::Point(2, 2));

      Delaunay trGenerator(in);
      trGenerator.Triangulate(dbgOutput);

      // 1st triangle
      auto iter = trGenerator.fbegin();
      auto area = iter.area(); 

      REQUIRE(area == 2); // == 0.5 * 4 !

      // 2nd triangle
      ++iter;
      area = iter.area(); // OPEN TODO:: move to mesh???

      REQUIRE(area == 2); // == 0.5 * 4 !

      // no 3rd triangle!
      ++iter;
      REQUIRE(iter == trGenerator.fend());

      // use postfix increment
      auto second = trGenerator.fbegin()++;
      area = (trGenerator.fbegin()++).area();
      REQUIRE(area == 2); // == 0.5 * 4 !
      REQUIRE(area == second.area());
   }

}


TEST_CASE("Usage of Points", "[trpp]")
{
   // prepare points
   std::vector<Delaunay::Point> pslgExamplePoints;

   pslgExamplePoints.push_back(Delaunay::Point(0, 0));
   pslgExamplePoints.push_back(Delaunay::Point(1, 0));
   pslgExamplePoints.push_back(Delaunay::Point(3, 0));
   pslgExamplePoints.push_back(Delaunay::Point(4, 0));
   pslgExamplePoints.push_back(Delaunay::Point(1.5, 1));
   pslgExamplePoints.push_back(Delaunay::Point(2.5, 1));
   pslgExamplePoints.push_back(Delaunay::Point(1.6, 1.5));
   pslgExamplePoints.push_back(Delaunay::Point(2.4, 1.5));
   pslgExamplePoints.push_back(Delaunay::Point(2, 2));
   pslgExamplePoints.push_back(Delaunay::Point(2, 3));

   // Sorting
   // std::sort(pslgExamplePoints.begin(), pslgExamplePoints.end()); ---> not working!

   std::sort(pslgExamplePoints.begin(), pslgExamplePoints.end(), Delaunay::OrderPoints());

   REQUIRE(std::is_sorted(pslgExamplePoints.begin(), pslgExamplePoints.end(), Delaunay::OrderPoints()));

   
   // OPEN TODO:: hashing support

   // ...

}


TEST_CASE("Usage of Triangulation Mesh", "[trpp]")
{
   // prepare points
   std::vector<Delaunay::Point> pslgExamplePoints;

   pslgExamplePoints.push_back(Delaunay::Point(0, 0));
   pslgExamplePoints.push_back(Delaunay::Point(1, 0));
   pslgExamplePoints.push_back(Delaunay::Point(3, 0));
   pslgExamplePoints.push_back(Delaunay::Point(4, 0));
   pslgExamplePoints.push_back(Delaunay::Point(1.5, 1));
   pslgExamplePoints.push_back(Delaunay::Point(2.5, 1));
   pslgExamplePoints.push_back(Delaunay::Point(1.6, 1.5));
   pslgExamplePoints.push_back(Delaunay::Point(2.4, 1.5));
   pslgExamplePoints.push_back(Delaunay::Point(2, 2));
   pslgExamplePoints.push_back(Delaunay::Point(2, 3));


   Delaunay trGenerator(pslgExamplePoints);
   trGenerator.Triangulate(dbgOutput);

   auto mesh = trGenerator.mesh();
   auto iterFirst = trGenerator.fbegin();
   auto iter = trGenerator.fend(); 
   
   SECTION("TEST XX.XX: Walk over triangles in the mesh")
   {
      iter = mesh.Lnext(iterFirst);
      iter = mesh.Lprev(iter);

      REQUIRE(iter == iterFirst); // roundtrip!

      iter = mesh.Onext(iterFirst);
      iter = mesh.Oprev(iter);
      
      REQUIRE(iter == iterFirst); // roundtrip!

      iter = mesh.Sym(iterFirst);

      // OPEN TODO::      
      //REQUIRE(); ....

   }

   SECTION("TEST XX.XX: Locate vertex in a mesh")
   {
      iter = mesh.locate(iterFirst.Org());

      REQUIRE(iter == iterFirst); // roundtrip!


      // OPEN TODO:: more tests for location

   }

   SECTION("TEST XX.XX: Find triangles around vertex in a mesh")
   {
      std::vector<int> ivv;     
      mesh.trianglesAroundVertex(iterFirst.Org(), ivv);

      REQUIRE(iterFirst.Org() == 0);
      REQUIRE(ivv.size() == 9);

      // OPEN TODO:: more checks 

   }

   // ... more to come...

}


TEST_CASE("Different triangulation algorithms", "[trpp]")
{
   std::vector<Delaunay::Point> pslgDelaunayInput;
   std::vector<Delaunay::Point> pslgDelaunaySegments;

   preparePLSGTestData(pslgDelaunayInput, pslgDelaunaySegments);

   Delaunay triGen(pslgDelaunayInput);

   triGen.Triangulate();
   auto triCountDefault = triGen.triangleCount();

   REQUIRE(triCountDefault == 13);

   triGen.setAlgorithm(DivideConquer);
   triGen.Triangulate();
   auto triCountDivideConquer = triGen.triangleCount();

   REQUIRE(triCountDefault == triCountDivideConquer);

   triGen.setAlgorithm(Incremental);
   triGen.Triangulate();
   auto triCountIncremental = triGen.triangleCount();

   REQUIRE(triCountDefault == triCountIncremental);

   triGen.setAlgorithm(Sweepline);
   triGen.Triangulate();
   auto triCountSweepline = triGen.triangleCount();

   REQUIRE(triCountDefault == triCountSweepline);

}


// --- eof ---
