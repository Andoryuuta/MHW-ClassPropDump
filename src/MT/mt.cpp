#include "mt.h"

// clang-format off

/// DTI Hash table AOB
/*
MHW-PC-64   : 48 8D 0D B7 78 F0 02    48 8D 0C C1    48 8B 01    48 85 C0    74 06
MHS2-PC-64  : 48 8D ?? ?? ?? ?? ??    48 8D 0C C1    48 8B 01    48 85 C0    74 18
UMVC3-PC-64 : 48 8D 0D 75 FE 90 00    48 8D 0C C1    48 8B 01    48 85 C0    74 20
Common-PC-64: 48 8D ?? ?? ?? ?? ?? 48 8D 0C C1 48 8B 01 48 85 C0 74 

DDA-PC-32   :                8B 04 8D ?? ?? ?? ??    8D 0C 8D ?? ?? ?? ??    85 C0 74
DDON-PC-32  : 0F B6 4E 1C    8B 04 8D ?? ?? ?? ??                            85 C0 74

PS4:
DDON-PS4-64 : 48 8D 15 37 B9 85 01    48 8B 0C C2    48 85 C9    74 25
*/


/// MT Type Group table AOB
/*
MHS2-PC-64-1     : 33 DB                4C 8D 15 C2 50 AF 00    44 8B CB    4C 8B D9    49 8B 02    4D 8B C3
MHW-PC-64-1      : 33 DB                4C 8D 15 12 36 E9 01    44 8B CB    4C 8B D9    49 8B 02    4D 8B C3 
common-pc-64-mt3 : 33 DB                4C 8D 15 ?? ?? ?? ??    44 8B CB    4C 8B D9    49 8B 02    4D 8B C3


UMBC3-PC-64-1    : 33 DB    4C 8B D9    4C 8D 15 1F 73 8C 00    44 8B CB    49 8B 02    4D 8B C3    4C 2B C0


common-mt2


MHS2-PC-64-2  : 66 83 FA 4B    73 0F    0F B7 C2        48 8D 0D 2D 50 AF 00                48 8B 04 C1    C3
MHW-PC-64-2   :    83 F9 4C    73 11    25 FF 0F 00 00  48 8D 0D 74 35 E9 01                48 8B 04 C1    C3
UMBC3-PC-64-2 : 66 83 FA 4B    73 0F                    48 8D 0D 90 72 8C 00    0F B7 C2    48 8B 04 C1    C3
*/

// clang-format on
