# This is for the features that allow the creating of filter folders in Visual Studio (source_group)
cmake_minimum_required (VERSION 3.10)

project (SpectrumAnalyser)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	find_package(OpenGL REQUIRED)
	find_package(Threads REQUIRED)
	find_package(X11 REQUIRED)
	set( gfxapi "GLFWApi")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	find_package(OpenGL REQUIRED)
	find_package(Threads REQUIRED)
	find_library(AUDIOTOOLBOX_LIBRARY AudioToolbox)
	set( gfxapi "GLFWApi")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	#set( gfxapi "DX11")
	set( gfxapi "GLFWApi")	
endif()


# vendor includes
set( vendor_dir ../Vendor )
include_directories( ${vendor_dir} )
include_directories( ${vendor_dir}/sokol )
include_directories( ${vendor_dir}/imgui-docking )
include_directories( ${vendor_dir}/chips )
include_directories( ${vendor_dir}/magic_enum/include )
include_directories( ${vendor_dir}/rzx-sdk )
include_directories( ${vendor_dir}/zlib )
include_directories( ${vendor_dir}/implot )
include_directories( ${vendor_dir}/json )

# other includes
include_directories( ../Shared )

include( ../Shared/CMakeShared.txt )

# compiler defines
add_compile_definitions( _CRT_SECURE_NO_WARNINGS )

# compiler options
set(CMAKE_BUILD_TYPE Debug)

# vendor source

# chips
file ( GLOB_RECURSE chips_src 
	${vendor_dir}/chips/*.h)

# imgui
set ( imgui_src_dir ${vendor_dir}/imgui-docking )

set ( imgui_src ${imgui_src_dir}/imgui.cpp 
	${imgui_src_dir}/imgui_demo.cpp 
	${imgui_src_dir}/imgui_draw.cpp 
	${imgui_src_dir}/misc/cpp/imgui_stdlib.cpp 
	${imgui_src_dir}/imgui_tables.cpp
	${imgui_src_dir}/imgui_widgets.cpp )
	
if(${gfxapi} STREQUAL "GLFWApi")
	set( imgui_backend_src ${imgui_src_dir}/backends/imgui_impl_glfw.cpp
	${imgui_src_dir}/backends/imgui_impl_opengl3.cpp )

	find_package(OpenGL REQUIRED)
	include_directories( ${OPENGL_INCLUDE_DIRS} )
	
	set( GLFW_BUILD_DOCS OFF CACHE BOOL  "GLFW lib only" )
	set( GLFW_INSTALL OFF CACHE BOOL  "GLFW lib only" )

	add_subdirectory( ${vendor_dir}/glfw ./glfw )

elseif(${gfxapi} STREQUAL "DX11")
	set( imgui_backend_src ${imgui_src_dir}/backends/imgui_impl_win32.cpp
	${imgui_src_dir}/backends/imgui_impl_dx11.cpp )
endif()

# implot
set ( implot_src ${vendor_dir}/implot/implot.h
	${vendor_dir}/implot/implot.cpp
	${vendor_dir}/implot/implot_demo.cpp
	${vendor_dir}/implot/implot_items.cpp
	${vendor_dir}/implot/implot_internal.h)

# rzx-lib
set ( rzxlib_src ${vendor_dir}/rzx-sdk/rzx.c
	${vendor_dir}/rzx-sdk/rzx.h)

# zlib
set ( zlib_src ${vendor_dir}/zlib/adler32.c
	${vendor_dir}/zlib/compress.c
	${vendor_dir}/zlib/crc32.c
	${vendor_dir}/zlib/deflate.c
	${vendor_dir}/zlib/infback.c
	${vendor_dir}/zlib/inffast.c
	${vendor_dir}/zlib/inflate.c
	${vendor_dir}/zlib/inftrees.c
	${vendor_dir}/zlib/trees.c
	${vendor_dir}/zlib/uncompr.c
	${vendor_dir}/zlib/zutil.c )

set ( vendor_src ${imgui_src} ${imgui_backend_src} ${implot_src} ${chips_src} ${rzxlib_src} ${zlib_src} )

# program source
file ( GLOB program_src 
	./*.cpp ./*.h
	Exporters/*.cpp Exporters/*.h
	GameViewers/*.cpp GameViewers/*.h
	Importers/*.cpp Importers/*.h
	SnapshotLoaders/*.cpp SnapshotLoaders/*.h
	Viewers/*.cpp Viewers/*.h)

if(${gfxapi} STREQUAL "GLFWApi")
	set ( platform_main GLFW/GLFWMain.cpp )
else()
	set ( platform_main Windows/WinMain.cpp )
endif()

add_executable (SpectrumAnalyser ${shared_src} ${program_src} ${platform_main} ${vendor_src} )

# This is to make the filter folders in Visual Studio, we need cmake 3.10 for this
source_group( TREE ${CMAKE_CURRENT_SOURCE_DIR}/${vendor_dir} PREFIX Vendor FILES ${vendor_src} )
source_group( TREE ${CMAKE_CURRENT_SOURCE_DIR}/../Shared PREFIX Shared FILES ${shared_src} )
source_group( TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX ZXSpectrum FILES ${program_src} ${platform_main})

set_target_properties( SpectrumAnalyser PROPERTIES CXX_STANDARD 17 )

#windows specific
if(${gfxapi} STREQUAL "DX11")

	# libraries
	target_link_libraries(${PROJECT_NAME} PRIVATE
		d3d11.lib dxgi.lib dxguid.lib uuid.lib
		kernel32.lib user32.lib
		comdlg32.lib advapi32.lib shell32.lib
		ole32.lib oleaut32.lib)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	# debugger working dir
	set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/../../Data/SpectrumAnalyser")

	set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
	
	if(${gfxapi} STREQUAL "GLFWApi")
		target_link_libraries(${PROJECT_NAME}
			glfw 
			${OPENGL_LIBRARIES} 
			${CMAKE_THREAD_LIBS_INIT}
			${X11_LIBRARIES}
			${CMAKE_DL_LIBS}
			)
	endif()
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	target_link_libraries(${PROJECT_NAME}
		glfw
		asound
		${OPENGL_LIBRARIES} 
		${CMAKE_THREAD_LIBS_INIT}
		${X11_LIBRARIES}
		${CMAKE_DL_LIBS}
		)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	target_link_libraries(${PROJECT_NAME}
		glfw
		${OPENGL_LIBRARIES} 
		${CMAKE_THREAD_LIBS_INIT}
		${CMAKE_DL_LIBS}
		${AUDIOTOOLBOX_LIBRARY}
		)
endif()
