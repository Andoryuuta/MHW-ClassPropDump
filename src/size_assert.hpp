#pragma once
#define assert_size(classobj,classobjsize) static_assert(sizeof(classobj) == classobjsize, #classobj" != "#classobjsize)