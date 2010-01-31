#ifndef _STD_LIB_GUARD_H_
#define _STD_LIB_GUARD_H_

// USAGE: Include this file prior to including any standard library headers.
//
// This file ensures that certain symbols are undefined prior to including
// standard library files. In some cases, certain header files define
// symbols that will get expanded in the standard library headers,
// and that is a bad thing.

// Need to undef reference here because 
#ifdef reference
#undef reference
#endif


#endif // _STD_LIB_GUARD_H_
