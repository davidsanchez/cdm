project(CDM)
#"project(TP1_plugin)
cmake_minimum_required(VERSION 2.8)
SET(CMAKE_DEBUG_POSTFIX "d")
if ( EXISTS ${MOS_PATH} )
         #SET(MOS_PATH "_X86")
elseif()
        SET(MOS_PATH "/MOS")
endif()

    
MESSAGE( STATUS "" )
MESSAGE( STATUS "==============================================" )
MESSAGE( STATUS "Now configuring serveur_OPCUA CMakeLists.txt" )
MESSAGE( STATUS "==============================================" )

    include_directories(${CMAKE_CURRENT_SOURCE_DIR})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/share)
    include_directories(${MOS_PATH}/include/MOS_Plugin)
#     include_directories(/home/operator/Meteo/YoctoLib.cpp.34560/Sources)
#     include_directories(/home/operator/Meteo/YoctoLib.cpp.34560/Sources/yapi)

     include_directories(Source)
     #include_directories(../../)
     #include_directories(../../VimbaCPP/Examples)
     include_directories(/usr/local/include/CCfits)




    link_directories(${MOS_PATH}/lib)
#    link_directories(/home/operator/Meteo/YoctoLib.cpp.34560/Binaries/linux/64bits)
    link_directories(${CMAKE_CURRENT_SOURCE_DIR})
    link_directories(/usr/lib)
    link_directories(/usr/local/lib)
        

IF (UNIX)
    INCLUDE_DIRECTORIES()
    ADD_DEFINITIONS(-Wall)
#    ADD_DEFINITIONS(-O2 -g -m64)
        ADD_DEFINITIONS(-O3 -m64 -fPIC -std=c++11)
    SET(PLATTFORM_LIBS pthread rt dl)
ENDIF (UNIX)


# create library
add_library(
         ${PROJECT_NAME}
        SHARED
        CDM.cpp
        /usr/include/ueye.h
        )
TARGET_LINK_LIBRARIES(${PROJECT_NAME}  PluginBase ueye_api)
#TARGET_LINK_LIBRARIES(${PROJECT_NAME}  m pthread stdc++ usb-1.0 yocto)
#TARGET_LINK_LIBRARIES(${PROJECT_NAME}  m pthread stdc++)
#TARGET_LINK_LIBRARIES(${PROJECT_NAME}  VimbaC VimbaCPP )
TARGET_LINK_LIBRARIES(${PROJECT_NAME}  opencv_core opencv_highgui opencv_imgproc )
TARGET_LINK_LIBRARIES(${PROJECT_NAME}  CCfits cfitsio )

set_target_properties( PROPERTIES DEBUG_POSTFIX "d")

# configure output path
INSTALL(TARGETS  ${PROJECT_NAME}
        RUNTIME DESTINATION ../bin
        LIBRARY DESTINATION ../plugins
        ARCHIVE DESTINATION ../lib
)
