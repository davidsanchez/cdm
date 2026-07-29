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

include_directories($ENV{MOS_PATH}/../include/MOS_Plugin)
include_directories($ENV{YOCTO_PATH}/Sources)
include_directories(../cdmdevice/src)
include_directories(../cdmdevice/include)
include_directories(/home/cdmmgr/LSTDeviceController/include/)


link_directories($ENV{MOS_LIB_PATH})
link_directories($ENV{YOCTO_PATH}/Binaries/linux/64bits)

    include_directories(${CMAKE_CURRENT_SOURCE_DIR})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/share)
    include_directories(/usr/include/libxml2)
    include_directories(/usr/local/lib/)
    include_directories(/usr/include/CCfits)
    include_directories(/home/cdmmgr/cdm/cdmdevice/include)
    include_directories(/home/cdmmgr/cdm/cdmdevice/src)

    include_directories(Source)

    include_directories(/home/cdmmgr/YoctoMeteo_cpp/Sources)
    include_directories(/home/cdmmgr/YoctoMeteo_cpp/Sources/yapi)

    link_directories(${MOS_PATH}/../lib)
    link_directories(${CMAKE_CURRENT_SOURCE_DIR})
    link_directories(/usr/lib)
    link_directories(/usr/local/lib)
    link_directories(/usr/lib64)
    link_directories(/home/cdmmgr/YoctoMeteo_cpp/Binaries/linux/64bits)
    link_directories(/home/cdmmgr/LSTDeviceController/lib)

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
        ../cdmdevice/src/CDMController.cpp
        ../cdmdevice/src/PluginCDM.cpp
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
target_link_libraries(
	${PROJECT_NAME}
	PluginBase
	DeviceController
#	libfftw3.a 
PROJECT (PluginAuxiliaryController)
CMAKE_MINIMUM_REQUIRED (VERSION 2.8)

if (CMAKE_BUILD_TYPE STREQUAL Debug)
      SET(CMAKE_DEBUG_POSTFIX "d")
endif (CMAKE_BUILD_TYPE STREQUAL Debug)


# convert build type to lower case
IF (CMAKE_BUILD_TYPE)
  STRING(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_TOLOWER)
ELSE(CMAKE_BUILD_TYPE)
  # set default configuration to release
  SET(CMAKE_BUILD_TYPE "Release")
  STRING(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_TOLOWER)
ENDIF(CMAKE_BUILD_TYPE)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ../lib/)

FIND_PACKAGE( LibXml2 )

    include_directories(${LIBXML2_INCLUDE_DIR})
    INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/include")
    include_directories($ENV{MOS_PATH}/../include/MOS_Plugin)
    include_directories(${MOS_PATH}/../include/MOS_Plugin)
    include_directories(../LSTDeviceController/include/)

# rajout pour specifique DriveController
    INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/include/Callbacks")

#    INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../sofa/include")
#    INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../JPLEphemx/include")
#    INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/fftw-3.3.8/api/")

    MESSAGE( STATUS " include = ${CMAKE_INSTALL_PREFIX}/include" )


    LINK_DIRECTORIES($ENV{MOS_PATH}/../lib)
    link_directories(${MOS_PATH}/../lib)
    link_directories(${MOS_PATH}/lib)
    link_directories(${MOS_PATH}/lib)

# rajout pour specifique DriveController
    LINK_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../LSTDeviceController/lib)
#    LINK_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../JPLEphemx/lib)
#    LINK_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/fftw-3.3.8/.libs)
#    LINK_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/sofa/lib)

add_library(
	${PROJECT_NAME}
        SHARED

	source/PluginAuxiliaryController.cpp 
	source/AuxiliaryController.cpp
	source/Callbacks/CB_arch.cpp
        source/Callbacks/CB_cdm.cpp
        source/Callbacks/CB_im.cpp
        source/Callbacks/CB_dm.cpp
        source/Callbacks/CB_encoders.cpp
        source/Callbacks/CB_calib.cpp
        source/Callbacks/CB_torque.cpp
        source/Callbacks/CB_power.cpp
        source/Callbacks/CB_AuxLog.cpp
        source/XMLParser.cpp
)
target_link_libraries(
	${PROJECT_NAME}
	PluginBase
	DeviceController
)


# configure output path
INSTALL(TARGETS  ${PROJECT_NAME}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION plugins
        ARCHIVE DESTINATION lib
)
