cmake_minimum_required (VERSION 3.8)

project(MHWClassPropDump VERSION "0.0.1")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(extern)

add_library(MHWClassPropDump SHARED
	"src/main.cpp"
	"src/SigScan.hpp"
	"src/SigScan.cpp"
	"src/Mt.hpp"
	"src/Mt.cpp"
	"src/size_assert.hpp"
)
  
target_include_directories (MHWClassPropDump PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
#target_include_directories (MHWClassPropDump PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/extern)
target_link_libraries(MHWClassPropDump PUBLIC fmt::fmt)
target_link_libraries(MHWClassPropDump PUBLIC spdlog::spdlog)



set_target_properties(MHWClassPropDump PROPERTIES COMPILE_FLAGS "/EHa")

