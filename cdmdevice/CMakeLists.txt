cmake_minimum_required(VERSION 3.9)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_DEBUG_POSTFIX "d")
set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-Wunused-variable")
#set (MOS_PATH "/data/rando/github/LST/mos-latest/MOS/MOS_X86_12.4.4a9")
#set (YOCTO_PATH "/data/rando/github/yoctometeo_cpp")

project(CDM)


if ( EXISTS ${CMAKE_INSTALL_PREFIX} )
    #SET(CMAKE_INSTALL_PREFIX "..")
else()
    SET(CMAKE_INSTALL_PREFIX "..")
endif()


find_package(OpenCV REQUIRED)
find_package(LibXml2 REQUIRED)
find_package(Boost COMPONENTS log log_setup REQUIRED)
find_package(ids_peak 1.10.0 REQUIRED)

include_directories(${LIBXML2_INCLUDE_DIR})
include_directories(/usr/include/CCfits)

include_directories($ENV{MOS_LIB_PATH}/../include/MOS_Plugin)
include_directories($ENV{YOCTO_PATH}/Sources)
include_directories(../cdmdevice/src)
include_directories(../cdmdevice/include)


link_directories($ENV{MOS_LIB_PATH})
link_directories($ENV{YOCTO_PATH}/Binaries/linux/64bits)


MESSAGE( STATUS "" )

MESSAGE( STATUS "==============================================" )
MESSAGE( STATUS "Now configuring server_OPCUA CMakeLists.txt" )
MESSAGE( STATUS "==============================================" )

    include_directories(${CMAKE_CURRENT_SOURCE_DIR})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/share)

    link_directories(${MOS_LIB_PATH})
    link_directories(${CMAKE_CURRENT_SOURCE_DIR})
    link_directories(/usr/lib)
    link_directories(/usr/local/lib)
    link_directories(/usr/lib64)
    link_directories(/home/cdmmgr/YoctoMeteo_cpp/Binaries/linux/64bits)

IF (UNIX)
    INCLUDE_DIRECTORIES()
    ADD_DEFINITIONS(-Wall)
#    ADD_DEFINITIONS(-O2 -g -m64)
    ADD_DEFINITIONS(-O3 -m64 -fPIC )
    ADD_DEFINITIONS(-DBOOST_LOG_DYN_LINK)
    SET(PLATTFORM_LIBS pthread rt dl)
ENDIF (UNIX)

#print includes
get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "incdir='${dir}'")
endforeach()
get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY LINK_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "linkdir='${dir}'")
endforeach()

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

target_link_libraries(${PROJECT_NAME} m pthread stdc++)
target_link_libraries(${PROJECT_NAME} PluginBase) # MOS
target_link_libraries(${PROJECT_NAME}  configXML ) #MOS
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})
target_link_libraries(${PROJECT_NAME} ${LibXml2_LIBS})
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} )
target_link_libraries(${PROJECT_NAME}  usb-1.0 yocto ) #YOCTO METEO
target_link_libraries(${PROJECT_NAME}  CCfits cfitsio ) # FITS files library
target_link_libraries(${PROJECT_NAME}  ids_peak ) # FITS files library


# configure output path
INSTALL(TARGETS  ${PROJECT_NAME}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION plugins
        ARCHIVE DESTINATION lib
)
