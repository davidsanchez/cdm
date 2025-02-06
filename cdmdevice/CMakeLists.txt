project(CDM)
cmake_minimum_required(VERSION 2.8)
SET(CMAKE_DEBUG_POSTFIX "d")

SET(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-Wunused-variable")

#if ( EXISTS ${MOS_PATH} )
    #SET(MOS_PATH "_X86")
#else()
    #SET(MOS_PATH "/MOS")
    #SET(MOS_PATH "/MOS_v4.0.2/")
    #SET(MOS_PATH "/MOS_v5.0.1/")
    #SET(MOS_PATH "/MOS_v7.0.0/")
    #SET(MOS_PATH "/MOS_test.1.3/")
#    SET(MOS_PATH "/MOS_v11.0.0/")
#endif()


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
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/share)
    include_directories(${MOS_PATH}/include/MOS_Plugin)
    include_directories(/usr/include/libxml2)
    include_directories(/usr/include/boost169)
    include_directories(/usr/include/CCfits)
    include_directories(/home/cdmmgr/cdm/include)
    include_directories(/home/cdmmgr/cdm/src)
#    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../devicecontroller/include)

    include_directories(Source)

    include_directories(/home/cdmmgr/YoctoMeteo_cpp/Sources)
    include_directories(/home/cdmmgr/YoctoMeteo_cpp/Sources/yapi)

    link_directories(${MOS_PATH}/lib)
    link_directories(${CMAKE_CURRENT_SOURCE_DIR})
    link_directories(/usr/lib)
    link_directories(/usr/local/lib)
    link_directories(/usr/lib64)
    link_directories(/home/cdmmgr/YoctoMeteo_cpp/Binaries/linux/64bits)
#    link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../devicecontroller/lib)


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
        ../src/CDM.cpp
        ../src/AsynchronousThread.cpp
        ../src/Camera.cpp
        ../src/Helper.cpp
        ../src/Meteo.cpp
        ../src/ImageAnalysis.cpp
        ../src/Circle/mystuff.h
        ../src/Circle/circle.h
        ../src/Circle/data.h
        ../src/Logging.cpp
        ../src/Config.cpp
        ../src/DatapointMonitor.cpp
        ../src/ControllerCB_changeStatus.cpp
        #ImageFITS.cpp
        )

#TARGET_LINK_LIBRARIES(${PROJECT_NAME}  m pthread stdc++)
TARGET_LINK_LIBRARIES(${PROJECT_NAME}  PluginBase) # MOS
TARGET_LINK_LIBRARIES(${PROJECT_NAME}  ueye_api) # Camera library
TARGET_LINK_LIBRARIES(${PROJECT_NAME}  opencv_core opencv_highgui opencv_imgproc ) #OpenCV
TARGET_LINK_LIBRARIES(${PROJECT_NAME}  CCfits cfitsio ) # FITS files library
TARGET_LINK_LIBRARIES(${PROJECT_NAME}  boost_log boost_log_setup ) # Boost
TARGET_LINK_LIBRARIES(${PROJECT_NAME}  usb-1.0 yocto ) # Yocto-Meteo
TARGET_LINK_LIBRARIES(${PROJECT_NAME}  curl ) # cURL
#TARGET_LINK_LIBRARIES(${PROJECT_NAME}  DeviceController ) # 

set_target_properties( PROPERTIES DEBUG_POSTFIX "d")

# configure output path
INSTALL(TARGETS  ${PROJECT_NAME}
        RUNTIME DESTINATION ./bin
        LIBRARY DESTINATION ./lib
        ARCHIVE DESTINATION ./lib
)
