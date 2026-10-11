cmake_minimum_required (VERSION 3.10)

project(MTDTIDump VERSION "0.0.1")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(MT_GAMES MHW MHS2 UMVC3 DDDA DDON)
set(MT_GAME "MHW" CACHE STRING "Game to compile for (one of: ${MT_GAMES})")
set_property(CACHE MT_GAME PROPERTY STRINGS ${MT_GAMES})
if(NOT MT_GAME IN_LIST MT_GAMES)
	message(FATAL_ERROR "Invalid MT_GAME '${MT_GAME}'")
endif()

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