cmake_minimum_required (VERSION 3.8)

project(MTDTIDump VERSION "0.0.1")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


SET(MT_GAME_MHW OFF CACHE BOOL "Compile for MHW")
SET(MT_GAME_MHS2 ON CACHE BOOL "Compile for MHS2")
configure_file(src/config.h.in config.h)

add_subdirectory(extern)

add_library(MTDTIDump SHARED
	"src/MT/MtCommon.hpp"
	"src/MT/MtDTI.hpp"
	"src/MT/MtDTI.cpp"
	"src/MT/MtDTIHashTable.hpp"
	"src/MT/MtObject.hpp"
	"src/MT/MtProperty.hpp"
	"src/MT/MtProperty.cpp"
	"src/MT/MtPropertyList.hpp"
	"src/main.cpp"
	"src/SigScan.hpp"
	"src/SigScan.cpp"
	"src/Mt.hpp"
	"src/size_assert.hpp"
	"src/DTIDumper.hpp"
	"src/DTIDumper.cpp"
	"src/crc.c"
	"src/crc.h"
)

# Include binary dir to include directory for the auto-generated config.h file.
target_include_directories (MTDTIDump PUBLIC ${PROJECT_BINARY_DIR})

target_include_directories (MTDTIDump PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
#target_include_directories (MTDTIDump PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/extern)

# Libraries.
target_link_libraries(MTDTIDump PUBLIC fmt::fmt)
target_link_libraries(MTDTIDump PUBLIC spdlog::spdlog)

set_target_properties(MTDTIDump PROPERTIES COMPILE_FLAGS "/EHa")

