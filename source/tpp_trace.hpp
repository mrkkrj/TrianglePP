 /** 
    @file  tpp_trace.hpp    
    @brief Macros for writing traces to a file. 
           Useful when debugging a GUI app. on Windows without console!
 */

#ifndef TRPP_TRACE_TO_FILE
#define TRPP_TRACE_TO_FILE

 
#ifdef TRIANGLE_DBG_TO_FILE
#   include <cstdio>

namespace tpp {
   extern FILE* g_debugFile;
}

// TR string
#   define TRACE(a) { if(tpp::g_debugFile) { fprintf(tpp::g_debugFile, "%s\n", a); fflush(tpp::g_debugFile); } }
    // TR string + integer 
#   define TRACE2i(a,b) { if(tpp::g_debugFile) { fprintf(tpp::g_debugFile, "%s%d\n", a, b); fflush(tpp::g_debugFile); } }
    // TR string + string 
#   define TRACE2s(a,b) { if(tpp::g_debugFile) { fprintf(tpp::g_debugFile, "%s%s\n", a, b); fflush(tpp::g_debugFile); } }
    // TR string + boolean 
#   define TRACE2b(a,b) { if(tpp::g_debugFile) { fprintf(tpp::g_debugFile, "%s%s\n", a, b ? "true " : "false"); fflush(tpp::g_debugFile); } }

#   define INIT_TRACE(a) { tpp::g_debugFile = fopen(a, "w"); \
                           if(!tpp::g_debugFile) std::cerr << "ERROR: Cannot open trace file: " << a << std::endl; }
#   define END_TRACE() { if(tpp::g_debugFile) { fclose(tpp::g_debugFile); } }
#else
#   define TRACE(a)
#   define TRACE2i(a,b) 
#   define TRACE2s(a,b) 
#   define TRACE2b(a,b) 
#   define INIT_TRACE(a) 
#   define END_TRACE() 
#endif // TRIANGLE_DBG_TO_FILE

#endif