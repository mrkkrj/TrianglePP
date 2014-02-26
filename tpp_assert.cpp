/*! \file assert.cpp
    \brief Implements a better 'Assert'
 */

#include <iostream>
#include <stdlib.h>

#if _WINDOWS
#include <cassert>
#endif

namespace tpp {

/*! \def MyAssertFunction
    \brief Function used by 'Assert' function in _DEBUG mode.
   
    Details.
*/
bool MyAssertFunction( bool b, const char* desc, int line, const char* file){
	// changed mrkkrj --
#if _WINDOWS
	(void)desc;
	(void)line;
	(void)file;
	assert(b); // use integration with Visual Studio!
	(void)b;
	return true;
#else
	// Original:
	if (b) return true;
	std::cerr << "\n\nAssertion Failure\n";
	std::cerr << "Description : " << desc << std::endl;
	std::cerr << "Filename    : " << file << std::endl;
	std::cerr << "Line No     : " << line << std::endl;
	exit(1);
#endif
}


} // end of namespace
