#pragma once

#if _WIN32 || _WIN64
    #if _WIN64
        #define ENV64BIT
    #else
        #define ENV32BIT
    #endif
#endif

#define assert_size(classobj, classobjsize)                                                                            \
    static_assert(sizeof(classobj) == classobjsize, "sizeof(" #classobj ") != " #classobjsize)

#ifdef ENV32BIT
    #define assert_size_32(classobj, classobjsize) assert_size(classobj, classobjsize)
#else
    #define assert_size_32(classobj, classobjsize)
#endif

#ifdef ENV64BIT
    #define assert_size_64(classobj, classobjsize) assert_size(classobj, classobjsize)
#else
    #define assert_size_64(classobj, classobjsize)
#endif