#This scripts will add all the cpp and h files under src and include folders, and
#assumes that your Ogre source code is in Dependencies/Ogre and that:
# In Windows you built Ogre into Dependencies/Ogre/build
# In Linux you built Release into Dependencies/Ogre/build/Release
# In Linux you built Debug into Dependencies/Ogre/build/Debug
# In MacOS you did the same as Linux, and didn't build Ogre as frameworks.
#
# If your source code is not at "Dependencies/Ogre"; you can use "mklink /D" to create
# a symbolic link to where the source code is located on Windows.
# In Unix, you can use "ln -s"

cmake_minimum_required( VERSION 3.3 )
project( imguiOgreNext )

set( EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}" )
include( CMake/Bootstrap.cmake )
include( CMake/Dependencies/OGRE.cmake )

setupOgre( OGRE_SOURCE, OGRE_BINARIES, OGRE_LIBRARIES )

file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.h")
if( APPLE )
    file(GLOB_RECURSE MMSOURCES "src/OgreCommon/System/OSX/*.mm")
endif()

file(GLOB IMGUI_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/imgui/*.cpp"
)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/imgui/)

if( APPLE )
    set( CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11" )
    set( CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++" )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -x objective-c++" )

    #Set the pluginFolder path in the plugins.cfg file.
    #Make a copy of the original one so it can be edited.
    execute_process( COMMAND "cp" "${OGRE_BINARIES}/bin/plugins.cfg" "/tmp/plugins.cfg" )
    execute_process( COMMAND bash -c "/usr/bin/sed -i '' 's#PluginFolder=#PluginFolder=${OGRE_BINARIES}/lib/macosx/${CMAKE_BUILD_TYPE}/#g' /tmp/plugins.cfg" )

    #file( GLOB_RECURSE RESOURCES ./src/*.storyboard )
    set( RESOURCES ${RESOURCES} ./bin/Data "./bin/Data/resources2.cfg" "/tmp/plugins.cfg" "${OGRE_SOURCE}/Samples/Media/" )
endif()

if( APPLE )
    add_executable( ${PROJECT_NAME} WIN32 MACOSX_BUNDLE ${SOURCES} ${HEADERS} ${IMGUI_SOURCES} ${MMSOURCES} ${RESOURCES} )
else()
    add_executable( ${PROJECT_NAME} WIN32 MACOSX_BUNDLE ${SOURCES} ${HEADERS} ${IMGUI_SOURCES} ${RESOURCES} )
endif()

target_link_libraries( ${PROJECT_NAME} ${OGRE_LIBRARIES} SDL2::SDL2 )

if( UNIX )
    target_link_libraries( ${PROJECT_NAME} dl )
endif()

if( APPLE )
    set_target_properties( ${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_ENABLE_BITCODE "NO" )
    set_target_properties( ${PROJECT_NAME} PROPERTIES RESOURCE "${RESOURCES}" )
    set_target_properties( ${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/src/OgreCommon/System/OSX/Resources/Info.plist )
    set( CMAKE_EXE_LINKER_FLAGS "
        -framework QuartzCore
        -framework Metal
        -framework OpenGL
        -framework cocoa
        -lfreetype
    ")


endif()
