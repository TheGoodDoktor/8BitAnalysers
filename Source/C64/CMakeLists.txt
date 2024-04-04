# This is for the features that allow the creating of filter folders in Visual Studio (source_group)
cmake_minimum_required (VERSION 3.10)

project (C64Analyser)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(APP_NAME C64Analyser)


if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	find_package(OpenGL REQUIRED)
	find_package(Threads REQUIRED)
	find_package(X11 REQUIRED)
	set( gfxapi "GLFWApi")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "" FORCE)
	set(MACOSX_BUNDLE_BUNDLE_NAME C64Analyser)
	find_package(OpenGL REQUIRED)
	find_package(Threads REQUIRED)
	find_library(AUDIOTOOLBOX_LIBRARY AudioToolbox)
	set( gfxapi "GLFWApi")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	#set( gfxapi "DX11")
	set( gfxapi "GLFWApi")	
endif()

# Put binary and configuration files to /bin subfolder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# other includes
include_directories( ../Shared )

include( ../Shared/CMakeShared.txt )
include( ../Vendor/CMakeVendor.txt )

# compiler defines
add_compile_definitions( _CRT_SECURE_NO_WARNINGS )

# compiler options
if (NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE Release)
endif ()

# program source
file ( GLOB program_src 
	./*.cpp ./*.c ./*.h
	GraphicsViewer/*.cpp GraphicsViewer/*.h
	IOAnalysis/*.cpp IOAnalysis/*.h 
	FileLoaders/*.cpp FileLoaders/*.h)

add_executable ( ${PROJECT_NAME} MACOSX_BUNDLE ${shared_src} ${program_src} ${vendor_src} )

# This is to make the filter folders in Visual Studio, we need cmake 3.10 for this
source_group( TREE ${CMAKE_CURRENT_SOURCE_DIR}/${vendor_dir} PREFIX Vendor FILES ${vendor_src} )
source_group( TREE ${CMAKE_CURRENT_SOURCE_DIR}/../Shared PREFIX Shared FILES ${shared_src} )
source_group( TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX C64 FILES ${program_src} )

#set_target_properties( ${PROJECT_NAME} PROPERTIES CXX_STANDARD 20 )
set_target_properties( ${PROJECT_NAME} PROPERTIES C_STANDARD 11 )

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
	set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/../../Data/C64Analyser")

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

    # This little macro lets you set any XCode specific property
    macro (set_xcode_property TARGET XCODE_PROPERTY XCODE_VALUE)
        set_property (TARGET ${TARGET} PROPERTY XCODE_ATTRIBUTE_${XCODE_PROPERTY} ${XCODE_VALUE})
    endmacro (set_xcode_property)

    set(APP_BUNDLE_IDENTIFIER "org.colourclash.C64Analyser")

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
  
    # Resource Files
    set(RESOURCE_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../Data/C64Analyser/imgui.ini
        #${CMAKE_CURRENT_SOURCE_DIR}/../../Data/C64Analyser/SALogo.png
        ${CMAKE_CURRENT_SOURCE_DIR}/../../Data/C64Analyser/RomInfo.json
        ${CMAKE_CURRENT_SOURCE_DIR}/../../Data/C64Analyser/Fonts/Cousine-Regular.ttf
    )
	# Copy ini file to /bin
	add_custom_command(TARGET ${APP_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${CMAKE_CURRENT_SOURCE_DIR}/../../Data/C64Analyser/imgui.ini
		$<TARGET_FILE_DIR:${APP_NAME}>
		)
  
    add_custom_command(TARGET ${APP_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Data/C64Analyser/Fonts
            $<TARGET_FILE_DIR:${APP_NAME}>/Fonts
            )
            
    add_custom_command(TARGET ${APP_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Data/C64Analyser/Lua
            $<TARGET_FILE_DIR:${APP_NAME}>/Lua
            )
        
    set_target_properties( C64Analyser PROPERTIES
        XCODE_ATTRIBUTE_OTHER_CODE_SIGN_FLAGS "--deep"
        BUNDLE True
        MACOSX_BUNDLE_INFO_STRING ${APP_BUNDLE_IDENTIFIER}
        MACOSX_BUNDLE_GUI_IDENTIFIER ${APP_BUNDLE_IDENTIFIER}
        MACOSX_BUNDLE_BUNDLE_NAME ${APP_BUNDLE_IDENTIFIER}
        MACOSX_BUNDLE_BUNDLE_NAME "${APP_NAME}"
        MACOSX_BUNDLE_BUNDLE_VERSION "0.1"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1"
        OUTPUT_NAME "C64 Analyser"
        RESOURCE "${RESOURCE_FILES}")
 
    set_xcode_property(${APP_NAME} PRODUCT_BUNDLE_IDENTIFIER ${APP_BUNDLE_IDENTIFIER})
	
    #set(APPS "\${CMAKE_INSTALL_PREFIX}/${APP_NAME}.app")
	#set(LIBS )
	#set(DIRS ${CMAKE_BINARY_DIR})
	#install(CODE "include(BundleUtilities)
	#fixup_bundle(\"${APPS}\" \"${LIBS}\" \"${DIRS}\")")
	#set(CPACK_GENERATOR "DRAGNDROP")
	#include(CPack)
endif()

