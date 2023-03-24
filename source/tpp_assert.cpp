 /** 
    @file  assert.cpp
    @brief Implements a better 'Assert'.
           Required in the reviver::dpoint inplementation.
 */

#include <iostream>
#include <stdlib.h>

#if _WINDOWS
#include <cassert>
#endif

namespace tpp {

   // test support
   bool g_disableAsserts = false;


bool MyAssertFunction( bool b, const char* desc, int line, const char* file)
{
    if (b) 
       return true;

    std::cerr << "\n\nAssertion Failure\n";
    std::cerr << "  Description : " << desc << std::endl;
    std::cerr << "  Filename    : " << file << std::endl;
    std::cerr << "  Line No     : " << line << std::endl;

    // changed mrkkrj --
#if _WINDOWS
    std::cerr << "  Calling WinAssert()... \n";
    assert(b); // use integration with Visual Studio!
    (void)b;
    return true;
#else
    // Original:
    exit(1);
#endif
}

} // end of namespace
