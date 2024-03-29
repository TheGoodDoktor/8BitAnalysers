# This is for the features that allow the creating of filter folders in Visual Studio (source_group)
cmake_minimum_required (VERSION 3.10)

project (CPCAnalyser)

set(APP_NAME CPCAnalyser)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	find_package(OpenGL REQUIRED)
	find_package(Threads REQUIRED)
	find_package(X11 REQUIRED)
	set( gfxapi "GLFWApi")
	# uncomment line below to support Raspberry Pi
	#add_compile_definitions( IMGUI_IMPL_OPENGL_ES2 )
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "" FORCE)
	set(MACOSX_BUNDLE_BUNDLE_NAME CPCAnalyser)
	find_package(OpenGL REQUIRED)
	find_package(Threads REQUIRED)
	find_library(AUDIOTOOLBOX_LIBRARY AudioToolbox)
	set( gfxapi "GLFWApi")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	#set( gfxapi "DX11")
	set( gfxapi "GLFWApi")	
endif()

# other includes
include_directories( ../Shared )

include( ../Shared/CMakeShared.txt )
include( ../Vendor/CMakeVendor.txt )

# compiler defines
add_compile_definitions( _CRT_SECURE_NO_WARNINGS )
add_compile_definitions( _SILENCE_ALL_CXX20_DEPRECATION_WARNINGS )

# compiler options
set(CMAKE_BUILD_TYPE Release)

# program source
file ( GLOB program_src 
	./*.cpp ./*.c ./*.h
	Shared/GamesList/*.cpp Shared/GamesList/*.h
	Exporters/*.cpp Exporters/*.h
	GameViewers/*.cpp GameViewers/*.h
	Importers/*.cpp Importers/*.h
	SnapshotLoaders/*.cpp SnapshotLoaders/*.h
	Viewers/*.cpp Viewers/*.h)

add_executable (CPCAnalyser MACOSX_BUNDLE ${shared_src} ${program_src} ${platform_main} ${vendor_src} )

# This is to make the filter folders in Visual Studio, we need cmake 3.10 for this
source_group( TREE ${CMAKE_CURRENT_SOURCE_DIR}/${vendor_dir} PREFIX Vendor FILES ${vendor_src} )
source_group( TREE ${CMAKE_CURRENT_SOURCE_DIR}/../Shared PREFIX Shared FILES ${shared_src} )
source_group( TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX CPC FILES ${program_src} ${platform_main})

set_target_properties( CPCAnalyser PROPERTIES CXX_STANDARD 20 )
set_target_properties( CPCAnalyser PROPERTIES C_STANDARD 11 )

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
	set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/../../Data/CPCAnalyser")

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
	install(TARGETS ${APP_NAME}
		BUNDLE DESTINATION . COMPONENT RunTime
		RUNTIME DESTINATION bin COMPONENT RunTime
		)
	add_custom_command(TARGET ${APP_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${CMAKE_CURRENT_SOURCE_DIR}/../../Data/CPCAnalyser/imgui.ini
		$<TARGET_FILE_DIR:${APP_NAME}>
		)
	set(APPS "\${CMAKE_INSTALL_PREFIX}/${APP_NAME}.app")
	set(LIBS )
	set(DIRS ${CMAKE_BINARY_DIR})
	install(CODE "include(BundleUtilities)
	fixup_bundle(\"${APPS}\" \"${LIBS}\" \"${DIRS}\")")
	set(CPACK_GENERATOR "DRAGNDROP")
	include(CPack)
endif()
