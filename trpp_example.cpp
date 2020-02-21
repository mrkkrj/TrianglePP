/*! \file trpp_example.cpp
    \brief example usage of the Triangle++ wrapper
 */

#include "tpp_interface.hpp"
#include <vector>
#include "trpp_example.h"


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


    // 2. triangulate with constraints
    bool withConstraints = true;

    for (int i = 0; i < 2; ++i)
    {
       if (i == 0)
       {
          // 2_a. triangulate with _default_ constraints (min angle = 20°)	
          trGenerator.Triangulate(withConstraints);
       }
       else
       {
          // 2_b. triangulate with _custom_ constraints (angle = 30.5°, area = 1.5)	
          trGenerator.setMinAngle(30.5f);
          trGenerator.setMaxArea(1.5f);
          trGenerator.Triangulate(withConstraints);
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

          // new vertices might have been added to enforce constraints!
          // (== Steiner points)
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
             x1 = sp1[1];
          }
       }
    }
}

// --- eof ---
