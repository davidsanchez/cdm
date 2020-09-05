#ifndef CDM_H_
#define CDM_H_

#include <cstring>
#include <fstream>
#include <sstream>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <ctime>
#include <iomanip> // Used for setprecision
#include <cmath>
#include <unistd.h> // TODO: why is this used?

#include "pluginsBase.h"	//TODO: move this to .h file? Was in .cpp before
#include "lappThread.h" // needed for MOS

#include <ueye.h> // IDS camera

#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/bimap.hpp>


namespace logging = boost::log;
namespace keywords = boost::log::keywords;


class DataAccessClientOPCUA;
class PluginsBase;
class SetDatapointThread : public LAPPThread
{
public:
    // TODO: refactor SetDatapointThread as a template instead of overloading?
    SetDatapointThread(DataAccessClientOPCUA *dataAccessClientOPCUA, std::string datapointName, int nameSpace,
                       std::vector<Byte> data)
    {
        m_dataAccessClientOPCUA = dataAccessClientOPCUA;
        m_datapointName = datapointName;
        m_nameSpace = nameSpace;
        m_data_vbyte = data;

        m_varType = varType::isVectorByte;
        start(&m_varType);
    };

    SetDatapointThread(DataAccessClientOPCUA *dataAccessClientOPCUA, std::string datapointName, int nameSpace,
                       std::string data)
    {
        m_dataAccessClientOPCUA = dataAccessClientOPCUA;
        m_datapointName = datapointName;
        m_nameSpace = nameSpace;
        m_data_str = data;

        m_varType = varType::isString;
        start(&m_varType);
    };

    SetDatapointThread(DataAccessClientOPCUA *dataAccessClientOPCUA, std::string datapointName, int nameSpace,
                       float data)
    {
        m_dataAccessClientOPCUA = dataAccessClientOPCUA;
        m_datapointName = datapointName;
        m_nameSpace = nameSpace;
        m_data_float = data;

        m_varType = varType::isFloat;
        start(&m_varType);
    };

    SetDatapointThread(DataAccessClientOPCUA *dataAccessClientOPCUA, std::string datapointName, int nameSpace,
                       double data)
    {
        m_dataAccessClientOPCUA = dataAccessClientOPCUA;
        m_datapointName = datapointName;
        m_nameSpace = nameSpace;
        m_data_double = data;

        m_varType = varType::isDouble;
        start(&m_varType);
    };

    SetDatapointThread(DataAccessClientOPCUA *dataAccessClientOPCUA, std::string datapointName, int nameSpace,
                       int data)
    {
        m_dataAccessClientOPCUA = dataAccessClientOPCUA;
        m_datapointName = datapointName;
        m_nameSpace = nameSpace;
        m_data_int = data;

        m_varType = varType::isInt;
        start(&m_varType);
    };

    ~SetDatapointThread(){};
    void *run(void *params)
    {
        //  std::string temString = m_datapointName + "._Done";
         //std::cout << "Params: " << *(static_cast<varType*>(params)) << std::endl;

        if (*(static_cast<varType *>(params)) == varType::isVectorByte)
            m_dataAccessClientOPCUA->setDatapoint(m_datapointName, m_nameSpace, m_data_vbyte);
        else if (*(static_cast<varType *>(params)) == varType::isString)
            m_dataAccessClientOPCUA->setDatapoint(m_datapointName, m_nameSpace, m_data_str);
        else if (*(static_cast<varType *>(params)) == varType::isInt)
            m_dataAccessClientOPCUA->setDatapoint(m_datapointName, m_nameSpace, m_data_int);
        else if (*(static_cast<varType *>(params)) == varType::isFloat)
            m_dataAccessClientOPCUA->setDatapoint(m_datapointName, m_nameSpace, m_data_float);
        else if (*(static_cast<varType *>(params)) == varType::isDouble)
            m_dataAccessClientOPCUA->setDatapoint(m_datapointName, m_nameSpace, m_data_double);
    };

private:
    int m_nameSpace;
    std::string m_datapointName;
    std::vector<Byte> m_data_vbyte;
    std::string m_data_str;
    int m_data_int;
    float m_data_float;
    double m_data_double;
    DataAccessClientOPCUA *m_dataAccessClientOPCUA;

    enum varType
    {
        isNULL = 0,
        isInt = 1, 
        isFloat = 2,
        isDouble = 3,
        isString = 4, 
        isVectorByte = 5
    };
    varType m_varType = varType::isNULL;
};

class CDM : public PluginsBase
{
public:
    int init(const std::string& chaine);
    int close();
    int cmd(const std::string& chaine, int commandStringAck, std::string& result);

    // new virtual methods appears with the version 3.0 of MOS
    int afterStart();
    int cmdAsynch(const std::string& command, int commandStringAck, const std::string& datapointName, int nameSpace,
                  std::string& result);

    // new virtual methods who replace the setAnay getAny methods  with the version 4.0 of MOS
    int get(const std::string& chaine, int commandStringAck, std::vector<boost::any>& tabValue);
    int set(const std::string& chaine, int commandStringAck, std::vector<boost::any>& tabValue);

    int Connect();
    int Disconnect();
    std::string Configure(int nPixelClock=216, double exposure=50, double fps=10, int gain=0, float n_images_integrate=1, std::string pixel_format="IS_CM_MONO8");
    int Comment(std::string comment);
    int GetImage();

//private:
    // Camera stuff
    HIDS hCam = (HIDS)0;
    SENSORINFO sensorinfo;
    CAMINFO camerainfo;
    HWND hWndDisplay = NULL; //DIB mode will be used for display
    char *pcImageMemory;
    int nMemoryId = 0;
    
    // Need to find out the memory size of the pixel and the colour mode
    int iColorMode = IS_CM_MONO8; //IS_CM_SENSOR_RAW16;
    int iBitsPerPixel = 8;        //16;
    int iWidth = 0;  // will be properly initialized with the sensor info struct information
    int iHeight = 0; // will be properly initialized with the sensor info struct information
    double dblFrameRateToSet = 10.0; // if set to 0.0 the max possible fps will be set
    IS_RECT rectAOI;
    int nRet;

    std::string comment = "";

    typedef boost::bimap< std::string, int > bimap;
    bimap pixel_formats;
    
    void insert_pixel_formats()
    {
        pixel_formats.insert({"IS_CM_MONO8", IS_CM_MONO8});
        pixel_formats.insert({"IS_CM_SENSOR_RAW8", IS_CM_SENSOR_RAW8});
        pixel_formats.insert({"IS_CM_SENSOR_RAW16", IS_CM_SENSOR_RAW16});
        
    }


private:

    std::string element_opcua_cdm_image = "MOS_Server.CDM.Image.Image_v";

    //TODO: Move some of the public stuff to private

    // Example of 2 methods
    // int userMethodStartAll(std::string argument);
    // int userMethodStopAll();

    // TODO: Refactor this. Need more of this for each device (Relay, Drive, Camera...). Template, polymorph or change function?
    // declare a new attribute
    DataAccessClientOPCUA *m_clientOpcUaRef = NULL;
    // declare a new method
    int connectOpcUa(std::string url);


};
#endif //  CDM_H_
