cmake_minimum_required (VERSION 3.8)

project(MTDTIDump VERSION "0.0.1")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

SET(MT_GAME_MHW OFF CACHE BOOL "Compile for MHW")
SET(MT_GAME_MHS2 OFF CACHE BOOL "Compile for MHS2")
SET(MT_GAME_UMVC3 ON CACHE BOOL "Compile for UMVC3")
SET(MT_GAME_DDDA OFF CACHE BOOL "Compile for DDDA")
SET(MT_GAME_DDON OFF CACHE BOOL "Compile for DDON")

configure_file(src/config.h.in config.h)

add_subdirectory(extern)

# add_library(MTDTIDumpLegacy SHARED
# 	"src_old/MT/MtCommon.hpp"
# 	"src_old/MT/MtDTI.hpp"
# 	"src_old/MT/MtDTI.cpp"
# 	"src_old/MT/MtDTIHashTable.hpp"
# 	"src_old/MT/MtObject.hpp"
# 	"src_old/MT/MtProperty.hpp"
# 	"src_old/MT/MtProperty.cpp"
# 	"src_old/MT/MtPropertyList.hpp"
# 	"src_old/main.cpp"
# 	"src_old/SigScan.hpp"
# 	"src_old/SigScan.cpp"
# 	"src_old/Mt.hpp"
# 	"src_old/size_assert.hpp"
# 	"src_old/DTIDumper.hpp"
# 	"src_old/DTIDumper.cpp"
# 	"src_old/crc.c"
# 	"src_old/crc.h"
# )

# # Include binary dir to include directory for the auto-generated config.h file.
# target_include_directories (MTDTIDumpLegacy PUBLIC ${PROJECT_BINARY_DIR})

# target_include_directories (MTDTIDumpLegacy PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
# #target_include_directories (MTDTIDumpLegacy PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/extern)

# # Libraries.
# target_link_libraries(MTDTIDumpLegacy PUBLIC fmt::fmt)
# target_link_libraries(MTDTIDumpLegacy PUBLIC spdlog::spdlog)

# set_target_properties(MTDTIDumpLegacy PROPERTIES COMPILE_FLAGS "/EHa")



# New version
add_library(MTDTIDump SHARED
	# "src/MT/MtCommon.hpp"
	# "src/MT/MtDTI.hpp"
	# "src/MT/MtDTI.cpp"
	# "src/MT/MtDTIHashTable.hpp"
	# "src/MT/MtObject.hpp"
	# "src/MT/MtProperty.hpp"
	# "src/MT/MtProperty.cpp"
	# "src/MT/MtPropertyList.hpp"
	# "src/DTIDumper.hpp"
	# "src/DTIDumper.cpp"

	
	"src/main.cpp"
	"src/sig_scan.cpp"
	"src/crc.c"
	"src/crc.h"
	"src/win_utils.cpp"
	"src/dumper.cpp"
	"src/mt/mt.cpp"
	"src/mt/mt_dti.cpp"
	"src/mt/game/UMVC3.cpp"
)

# Include binary dir to include directory for the auto-generated config.h file.
target_include_directories (MTDTIDump PUBLIC ${PROJECT_BINARY_DIR})

target_include_directories (MTDTIDump PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories (MTDTIDump PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/extern)

# Libraries.
target_link_libraries(MTDTIDump PUBLIC fmt::fmt)
target_link_libraries(MTDTIDump PUBLIC spdlog::spdlog)

set_target_properties(MTDTIDump PROPERTIES COMPILE_FLAGS "/EHa")