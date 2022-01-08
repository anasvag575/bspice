#ifndef __BASE_TYPES_H
#define __BASE_TYPES_H

/* Integer type used in indices */
typedef int IntTp;
typedef double FpTp;

#ifdef BSPICE_EIGEN_USE_STLMAPS
    /* STL - Chain hashtable implementation */
    #include <unordered_map>
    typedef std::unordered_map<std::string, IntTp> hashmap_str_t;
#else

    /* Custom hashtable implementation, very good for <String - Integer> */
    #include "robin_map.h"
    typedef tsl::robin_map<std::string, IntTp> hashmap_str_t;
#endif

#endif // __BASE_TYPES_H //
