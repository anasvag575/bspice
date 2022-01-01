#ifndef __BASE_TYPES_H
#define __BASE_TYPES_H

/* Integer type used in indices */
typedef int IntTp;
typedef double FpTp;

/* Using TSL implementantion of Robin map */	// TODO - Make it with IFDEF
#include "robin_map.h"
typedef tsl::robin_map<std::string, IntTp> hashmap_str_t;

///* STL - Chain hashtable implementation */
//typedef std::unordered_map<std::string, IntTp> hashmap_str_t;


#endif // __MATRIX_TYPES_H //
