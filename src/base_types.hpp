#ifndef __BASE_TYPES_H
#define __BASE_TYPES_H

//! Integer size used inside BSPICE.
typedef int IntTp;

//! Floating point accuracy used inside BSPICE.
typedef double FpTp;

#ifdef BSPICE_EIGEN_USE_STLMAPS
    #include <unordered_map>

    //! STL - Chain hashtable implementation.
    typedef std::unordered_map<std::string, IntTp> hashmap_str_t;
#else
    #include "robin_map.h"

    //! Custom hashtable implementation, very good for <String - Integer>.
    typedef tsl::robin_map<std::string, IntTp> hashmap_str_t;
#endif

#endif // __BASE_TYPES_H //
