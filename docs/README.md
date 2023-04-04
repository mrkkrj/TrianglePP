## Usage Example:

For usage patterns see the examples in the *trpp_example.cpp* source file. The interface of the *Tpp*-wrapper is defined in the *tpp_inteface.hpp* header file. 
Basic usage example is shown in the code snippet below:

    // prepare input
    std::vector<Delaunay::Point> delaunayInput = { ... };       

    // use standard triangulation
    Delaunay trGenerator(delaunayInput);
    trGenerator.Triangulate();

That's all! Now let have a look at the generated triangulation.

### Iterating over results

    // iterate over triangles
    for (FaceIterator fit = trGenerator.fbegin(); fit != trGenerator.fend(); ++fit)
    {
        int vertexIdx1 = fit.Org(); 
        int vertexIdx2 = fit.Dest();
        int vertexIdx3 = fit.Apex();

        // access point's cooridinates: 
        double x1 = delaunayInput[vertexIdx1][0];
        double y1 = delaunayInput[vertexIdx1][1];
    }

Note that the *vertexIndex* is relative to the input vertex vector! In case of Steiner points (i.e. points which were added by the algorithm) *vertexIndex* will be -1. 
In that case you can access Steiner points coordinates using an optional parameter, ase shown below:

        int vertexIdx1 = fit.Org(); 
        if (vertexIdx1 < 0)
        {
            Delaunay::Point sp;
            f.Org(&sp);
        }

        // etc..

### foreach() style loop

You can also use the *foreach()* style loop as shown below:

    for (const auto& f : trGenerator.faces())
    {
        int vertexIdx1 = f.Org(); 
        int vertexIdx2 = f.Dest();
        int vertexIdx3 = f.Apex();

        // etc...
    }

### Iterating using mesh indexes

t.b.c. ...

### Other iterators

t.b.c. ...

### Mesh walking

t.b.c. ...

### Quality constraints

t.b.c. ...

### Segment constraints

t.b.c. ...

### Voronoi diagrams

t.b.c. ...


## Traces and Logs

If compiled with *TRIANGLE_DBG_TO_FILE* define, debug traces will be written to the *./triangle.out.txt* file.

t.b.c. ...


## Example TrPP data files

You can write and read ASCII files containing point and segement definitions using Triangle++ methods. Some examples are stored in the *tppDataFiles* directory. Documentation for the used formats:

t.b.c. ...


## Theory:

![Triangle logo](../T.gif) 


The original *Triangle* library documentation can be found at: http://www.cs.cmu.edu/~quake/triangle.html. The library was a **winner** of the 2003 James Hardy Wilkinson Prize in Numerical Software (sic!).

For backgroud info on the original implementation see "*Triangle: Engineering a 2D Quality Mesh Generator and Delaunay Triangulator*" by J.P. Shewchuk: http://www.cs.cmu.edu/~quake-papers/triangle.ps (or the local copy listed below!).

Algorithm used for DCT construction: "*Fast segment insertion and incremental construction of constrained Delaunay triangulations*", Shewchuk, J.R., Brown, B.C., Computational Geometry, Volume 48, Issue 8, September 2015, Pages 554-574 - https://doi.org/10.1016/j.comgeo.2015.04.006


## Extra Local Files:
 
1. **triangle.pdf** - J.R. Shevchuk, *"Triangle: Engineering a 2D Quality Mesh Generator and Delaunay Triangulator"*, copy of: 'http://www.cs.cmu.edu/~quake-papers/triangle.ps'

2. **TriLib README.txt** - Docs extracted form Triangle's sources
