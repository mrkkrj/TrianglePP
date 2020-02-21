/*! \file trpp_example.cpp
    \brief example usage of the Triangle++ wrapper
 */

#include "tpp_interface.hpp"

#include <vector>
#include <iostream>


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

    // 2. triangulate with constraints
    bool withConstraints = true;
    bool dbgOutput = true;
    int nrOfConstraintTests = 3;

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

    for (int i = 0; i < nrOfConstraintTests; ++i)
    {
       if (i == 0)
       {
          std::cout << "\nTEST: default constraints (min angle = 20°) \n";
          
          trGenerator.Triangulate(withConstraints, dbgOutput);
       }
       else if (i == 1)
       {
          std::cout << "\nTEST: custom constraints (angle = 30.5°, area = 5.5) \n";
          
          trGenerator.setMinAngle(30.5f);
          trGenerator.setMaxArea(5.5f);
          trGenerator.Triangulate(withConstraints, dbgOutput);
       }
       else if (i == 2)
       {
          // bug hunt - 44 deg results in an endless loop 
          //  --> triangles too tiny for the floating point precision!
          std::cout << "\nTEST: custom constraints (angle = 44°) \n";
                    
          trGenerator.setMinAngle(44.0f);
          trGenerator.Triangulate(withConstraints, dbgOutput);
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

    std::cout << "TEST: completed ---" << std::endl;
}

// --- eof ---
