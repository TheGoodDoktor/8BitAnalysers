cmake_minimum_required (VERSION 2.8.11)
project (SpectrumAnalyser)

# vendor includes
set( vendor_dir ../Vendor )
include_directories( ${vendor_dir}/sokol )
include_directories( ${vendor_dir}/imgui-docking )
include_directories( ${vendor_dir}/chips )
include_directories( ${vendor_dir}/magic_enum/include )
include_directories( ${vendor_dir}/rzx-sdk )
include_directories( ${vendor_dir}/zlib )
include_directories( ${vendor_dir}/ImPlot )
include_directories( ${vendor_dir}/json )

#compiler defines

#compiler options
set(CMAKE_BUILD_TYPE Debug)

# vendor source
set ( imgui_src_dir ${vendor_dir}/imgui-docking )

#imgui
set ( imgui_src ${imgui_src_dir}/imgui.cpp 
	${imgui_src_dir}/imgui_demo.cpp 
	${imgui_src_dir}/imgui_draw.cpp 
	${imgui_src_dir}/imgui_stdlib.cpp 
	${imgui_src_dir}/imgui_tables.cpp
	${imgui_src_dir}/imgui_widgets.cpp )

set ( vendor_src ${imgui_src} GameConfig.cpp
	GameData.cpp
	GlobalConfig.cpp 
	IOAnalysis.cpp
	MemoryHandlers)

# program source
file ( GLOB_RECURSE program_src 
	./*.cpp )

add_executable (SpectrumAnalyser ${program_src} ${vendor_src} )