## Usage Example:

For usage patterns see the examples in the *trpp_example.cpp* source file. The interface of the *Tpp*-wrapper is defined in the *tpp_inteface.hpp* header file. 
Basic usage example is shown in the code snippet below:

    // prepare input
    std::vector<Delaunay::Point> delaunayInput;
    
    delaunayInput.push_back(Delaunay::Point(0,0));
    delaunayInput.push_back(Delaunay::Point(1,1));
    delaunayInput.push_back(Delaunay::Point(0,2));
    delaunayInput.push_back(Delaunay::Point(3,3));

    // use standard triangulation
    Delaunay trGenerator(delaunayInput);
    trGenerator.Triangulate();

    // iterate over triangles
    for (FaceIterator fit = trGenerator.fbegin(); fit != trGenerator.fend(); ++fit)
    {
        int keypointIdx1 = fit.Org(); 
        int keypointIdx2 = fit.Dest();
        int keypointIdx3 = fit.Apex();

        // access point's cooridinates: 
        double x1 = delaunayInput[keypointIdx1][0];
        double y1 = delaunayInput[keypointIdx1][1];
    }


Note that the vertexIndex is relative to the input vertex vector!


You can also use the *foreach()* style loop as shown below:

    for (const auto& f : trGenerator.faces())
    {
        int keypointIdx1 = f.Org(); 
        int keypointIdx2 = f.Dest();
        int keypointIdx3 = f.Apex();

        // etc...
    }

## Example TrPP data files

You can write and read ASCII files containing point and segement definitions. Some examples are stored in the *tppDataFiles* directory. Documentation for the used formats:

 - OPEN TODO:: .... (coming soon...)

## Docs Files:
 
1. **triangle.pdf** - J.R. Shevchuk, *"Triangle: Engineering a 2D Quality Mesh Generator and Delaunay Triangulator"*, copy of: 'http://www.cs.cmu.edu/~quake-papers/triangle.ps'

2. **TriLib README.txt** - Docs extracted form Triangle's sources

