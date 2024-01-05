cmake_minimum_required (VERSION 3.8)

project(MTDTIDump VERSION "0.0.1")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

SET(MT_GAME_MHW ON BOOL "Compile for MHW")
SET(MT_GAME_MHS2 OFF BOOL "Compile for MHS2")
SET(MT_GAME_UMVC3 OFF BOOL "Compile for UMVC3")
SET(MT_GAME_DDDA OFF BOOL "Compile for DDDA")
SET(MT_GAME_DDON OFF BOOL "Compile for DDON")

configure_file(src/config.h.in config.h)

add_subdirectory(extern)

add_library(MTDTIDump SHARED
	"src/main.cpp"
	"src/sig_scan.cpp"
	"src/crc.c"
	"src/crc.h"
	"src/log.cpp"
	"src/win_utils.cpp"
	"src/safe_thread_calls.cpp"
	"src/dumper.cpp"
	"src/mt/mt.cpp"
	"src/mt/mt_dti.cpp"
)

# Include binary dir to include directory for the auto-generated config.h file.
target_include_directories (MTDTIDump PUBLIC ${PROJECT_BINARY_DIR})

target_include_directories (MTDTIDump PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories (MTDTIDump PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/extern)

# Libraries.
target_link_libraries(MTDTIDump PUBLIC fmt::fmt)
target_link_libraries(MTDTIDump PUBLIC spdlog::spdlog)

set_target_properties(MTDTIDump PROPERTIES COMPILE_FLAGS "/EHa")