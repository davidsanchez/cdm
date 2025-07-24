project(CDM)
cmake_minimum_required(VERSION 2.8)
SET(CMAKE_DEBUG_POSTFIX "d")

SET(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-Wunused-variable")



if ( EXISTS ${CMAKE_INSTALL_PREFIX} )
    #SET(CMAKE_INSTALL_PREFIX "..")
else()
    SET(CMAKE_INSTALL_PREFIX "..")
endif()

MESSAGE( STATUS "" )

MESSAGE( STATUS "==============================================" )
MESSAGE( STATUS "Now configuring server_OPCUA CMakeLists.txt" )
MESSAGE( STATUS "==============================================" )

    include_directories(${CMAKE_CURRENT_SOURCE_DIR})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/share)
    
    include_directories($ENV{MOS_PATH}/../include/MOS_Plugin)
    include_directories(${MOS_PATH}/../include/MOS_Plugin)
    include_directories(/usr/include/libxml2)
    include_directories(/usr/local/lib/)
    include_directories(/usr/include/CCfits)
    include_directories(/home/cdmmgr/cdm/cdmdevice/include)
    include_directories(/home/cdmmgr/cdm/cdmdevice/src)

    include_directories(Source)

    include_directories(/home/cdmmgr/YoctoMeteo_cpp/Sources)
    include_directories(/home/cdmmgr/YoctoMeteo_cpp/Sources/yapi)

    link_directories(${MOS_PATH}/lib)
    link_directories(${CMAKE_CURRENT_SOURCE_DIR})
    link_directories(/usr/lib)
    link_directories(/usr/local/lib)
    link_directories(/usr/lib64)
    link_directories(/home/cdmmgr/YoctoMeteo_cpp/Binaries/linux/64bits)

IF (UNIX)
    INCLUDE_DIRECTORIES()
    ADD_DEFINITIONS(-Wall)
#    ADD_DEFINITIONS(-O2 -g -m64)
    ADD_DEFINITIONS(-O3 -m64 -fPIC -std=c++11)
    ADD_DEFINITIONS(-DBOOST_LOG_DYN_LINK)
    SET(PLATTFORM_LIBS pthread rt dl)
ENDIF (UNIX)

# Create library
add_library(
         ${PROJECT_NAME}
        SHARED
        #/usr/include/ueye.h
        ../cdmdevice/src/CDM.cpp
        ../cdmdevice/src/AsynchronousThread.cpp
        ../cdmdevice/src/Camera.cpp
        ../cdmdevice/src/Helper.cpp
        ../cdmdevice/src/Meteo.cpp
        ../cdmdevice/src/ImageAnalysis.cpp
        ../cdmdevice/src/Circle/mystuff.h
        ../cdmdevice/src/Circle/circle.h
        ../cdmdevice/src/Circle/data.h
        ../cdmdevice/src/Logging.cpp
        ../cdmdevice/src/ConfigCDM.cpp
        ../cdmdevice/src/DatapointMonitor.cpp
        ../cdmdevice/src/ControllerCB_changeStatus.cpp
        #ImageFITS.cpp
        )

#TARGET_LINK_LIBRARIES(${PROJECT_NAME}  m pthread stdc++)
TARGET_LINK_LIBRARIES(${PROJECT_NAME}  PluginBase) # MOS
TARGET_LINK_LIBRARIES(${PROJECT_NAME}  ueye_api) # Camera library
TARGET_LINK_LIBRARIES(${PROJECT_NAME}  opencv_core opencv_highgui opencv_imgproc ) #OpenCV
TARGET_LINK_LIBRARIES(${PROJECT_NAME}  CCfits cfitsio ) # FITS files library
TARGET_LINK_LIBRARIES(${PROJECT_NAME}  boost_log boost_log_setup ) # Boost
TARGET_LINK_LIBRARIES(${PROJECT_NAME}  usb-1.0 yocto ) # Yocto-Meteo
#TARGET_LINK_LIBRARIES(${PROJECT_NAME}  curl ) # cURL
TARGET_LINK_LIBRARIES(${PROJECT_NAME}  configXML ) # 



set_target_properties( PROPERTIES DEBUG_POSTFIX "d")


find_package (OpenCV 4.0.0 REQUIRED)
include_directories ("/usr/include/opencv4/")



# configure output path
INSTALL(TARGETS  ${PROJECT_NAME}
        RUNTIME DESTINATION ../bin
        LIBRARY DESTINATION ../plugins
        ARCHIVE DESTINATION ../lib
)
