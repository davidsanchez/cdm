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
#include "test_asynchroneThread.h"

//#include <ueye.h> // IDS camera

// #include <opencv2/core/core.hpp>
// #include <opencv2/highgui/highgui.hpp>
// #include <opencv2/video/video.hpp>
// #include <opencv2/opencv.hpp>

#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/algorithm/string.hpp>
//#include <boost/bimap.hpp>
//#include <boost/assign.hpp>


namespace logging = boost::log;
namespace keywords = boost::log::keywords;


class DataAccessClientOPCUA;
class PluginsBase;
class TestAsynchroneThread;
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
                       std::vector<std::string> data)
    {
        m_dataAccessClientOPCUA = dataAccessClientOPCUA;
        m_datapointName = datapointName;
        m_nameSpace = nameSpace;
        m_data_vstring = data;

        m_varType = varType::isVectorString;
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
        else if (*(static_cast<varType *>(params)) == varType::isVectorString)
            m_dataAccessClientOPCUA->setDatapoint(m_datapointName, m_nameSpace, m_data_vstring);
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
    DataAccessClientOPCUA *m_dataAccessClientOPCUA;

    int m_data_int;
    float m_data_float;
    double m_data_double;
    std::vector<Byte> m_data_vbyte;
    std::vector<std::string> m_data_vstring;
    std::string m_data_str;  

    enum varType
    {
        isNULL = 0,
        isInt = 1, 
        isFloat = 2,
        isDouble = 3,
        isString = 4, 
        isVectorByte = 5, 
        isVectorString = 6
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

    int Comment(std::string comment);
    std::string comment = "TestingDelete";
/* 
    int Connect();
    int Disconnect();
    std::string Configure(int nPixelClock=216, double exposure=1, double fps=1, int gain=0, std::string pixel_format="IS_CM_MONO8");
    int Comment(std::string comment);
    int GetImage();
    int GetMultipleImages(int n_images);
    int Start();
    int Stop();

//private:
    // Camera stuff
    HIDS hCam = (HIDS)0;
    SENSORINFO sensorinfo;
    CAMINFO camerainfo;
    HWND hWndDisplay = NULL; //DIB mode will be used for display
    char *pcImageMemory = NULL;
    int nMemoryId = 0;

    uint formatID = 36;

    
    // Need to find out the memory size of the pixel and the colour mode
    int iColorMode = IS_CM_MONO8; //IS_CM_SENSOR_RAW16;
    int iBitsPerPixel = 8;        //16;
    int iWidth = 0;  // will be properly initialized with the sensor info struct information
    int iHeight = 0; // will be properly initialized with the sensor info struct information
    double dblFrameRateToSet = 10.0; // if set to 0.0 the max possible fps will be set
    IS_RECT rectAOI;
    int nRet;

    int m_active=0;

    typedef boost::bimap< std::string, int > bimap;
    bimap pixel_formats;
    
    typedef boost::bimap< std::string, int > bimap;
    const bimap pixel_formats = boost::assign::list_of< bimap::relation >
    ( "IS_CM_MONO8", IS_CM_MONO8 )
    ( "IS_CM_SENSOR_RAW8", IS_CM_SENSOR_RAW8 )
    ( "IS_CM_SENSOR_RAW16", IS_CM_SENSOR_RAW16 );

 */

private:

    std::string element_opcua_cdm_image = "MOS_Server.CDM.Image.Image_v";
    TestAsynchroneThread* m_testThread;

    //TODO: Move some of the public stuff to private

    // Example of 2 methods
    // int userMethodStartAll(std::string argument);
    // int userMethodStopAll();

    // TODO: Refactor this. Need more of this for each device (Relay, Drive, Camera...). Template, polymorph or change function?
    // declare a new attribute
    DataAccessClientOPCUA *m_clientOpcUaRef = NULL;
    // declare a new method
    int connectOpcUa(std::string url);

    // declare a  new method
    int connectOpcUa_Drive(std::string url);
    int connectOpcUa_Relay(std::string url);
    int connectOpcUa_ECC(std::string url);

    
    // declare a new attribute
    DataAccessClientOPCUA* m_clientOpcUaRef_Drive=NULL;
    DataAccessClientOPCUA* m_clientOpcUaRef_Relay=NULL;
    DataAccessClientOPCUA* m_clientOpcUaRef_ECC=NULL;

    std::string StarName = "None";
    //const std::string imagePath = "/home/lstoperator/CDM/images/";
    const std::string fitsPath = "/home/lstoperator/CDM/fits/";
    const std::string remoteImagePathPrefix = "/fefs/home/lapp/CDM_Images/";

    int LED_intensity=0;
    bool OARL_state=0;
    double zenith = 0;
    double azimuth = 0;
    double offset_azimuth = 0;
    double offset_zenith = 0;
    double RA = 0;
    double DEC = 0;

    double get_RA() { return CDM::RA; }
    double get_DEC() { return CDM::DEC; }
    double get_Azimuth() { return CDM::azimuth; }
    double get_Zenith() { return CDM::zenith; }
    //double get_exposure() { return CDM::exposure; }
    double get_OffsetAzimuth() { return CDM::offset_azimuth; }
    double get_OffsetZenith() { return CDM::offset_zenith; }
    std::string get_StarName() { return CDM::StarName; }
    int get_LED_intensity() {return CDM::LED_intensity;}
    bool get_OARL_state() {return CDM::OARL_state;}


    struct Image_header_info
    {
        std::string starName;
        //double exposure;
        double RA;
        double DEC;
        double zenith;
        double azimuth;
        //double offset_azimuth;
        //double offset_zenith;
        int LED_intensity;
        bool OARL_state;
    }image_header_info;

    double acquire_RA();
    double acquire_DEC();
    double acquire_Azimuth();
    double acquire_Zenith();
    int acquire_LED_intensity();
    bool acquire_OARL_state();

};
#endif //  CDM_H_
