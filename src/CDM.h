#ifndef CDM_H_
#define CDM_H_

#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/video/video.hpp>

#include "lappThread.h" // needed for MOS
#include <ueye.h>       // Camera library (IDS)

class DataAccessClientOPCUA;
class PluginsBase;
class SetDatapointThread : public LAPPThread
{
public:
    SetDatapointThread(DataAccessClientOPCUA *dataAccessClientOPCUA, std::string datapointName, int nameSpace,
                       std::vector<Byte> data)
    {
        m_dataAccessClientOPCUA = dataAccessClientOPCUA;
        m_datapointName = datapointName;
        m_nameSpace = nameSpace;
        m_data = data;

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

    ~SetDatapointThread(){};
    void *run(void *params)
    {
        //  std::string temString = m_datapointName + "._Done";

        // std::cout << "Params: " << *(static_cast<varType*>(params)) << std::endl;

        if (*(static_cast<varType *>(params)) == varType::isVectorByte)
            m_dataAccessClientOPCUA->setDatapoint(m_datapointName, m_nameSpace, m_data);

        else if (*(static_cast<varType *>(params)) == varType::isString)
            m_dataAccessClientOPCUA->setDatapoint(m_datapointName, m_nameSpace, m_data_str);
    };

private:
    int m_nameSpace;
    std::string m_datapointName;
    std::vector<Byte> m_data;
    std::string m_data_str;
    DataAccessClientOPCUA *m_dataAccessClientOPCUA;

    enum varType
    {
        isNULL = 0,
        isVectorByte = 1,
        isString = 2
    };
    varType m_varType = varType::isNULL;
};

class CDM : public PluginsBase
{
public:
    int init(std::string chaine);
    int close();
    int cmd(std::string chaine, int commandStringAck, std::string *result);

    // new virtual methods appears with the version 3.0 of MOS
    int afterStart();
    int cmdAsynch(std::string command, int commandStringAck, std::string datapointName, int nameSpace,
                  std::string *result);

    // new virtual methods who replace the setAnay getAny methods  with the version 4.0 of
    // MOS
    int get(std::string chaine, int commandStringAck, std::vector<boost::any> *tabValue);
    int set(std::string chaine, int commandStringAck, std::vector<boost::any> tabValue);

    std::string writeImage(cv::Mat image);

    double get_RA() { return CDM::RA; }
    double get_DEC() { return CDM::DEC; }
    double get_Azimuth() { return CDM::azimuth; }
    double get_Zenith() { return CDM::zenith; }
    double get_exposure() { return CDM::exposure; }

    double get_OffsetAzimuth() { return CDM::offset_azimuth; }
    double get_OffsetZenith() { return CDM::offset_zenith; }
    std::string get_StarName() { return CDM::StarName; }

private:
    // int userMethodStartAll(std::string argument);
    // int userMethodStopAll();

    /* // IDS camera stuff
    VmbErrorType    err         = VmbErrorSuccess;
    char *          pCameraID   = NULL;             // The ID of the camera to use
    const char *    pFileName   = NULL;             // The filename for the bitmap to
    save bool            bPrintHelp  = false;            // Output help? int i; //
    Counter for some iteration char *          pParameter;                     // The
    command line parameter */

    HIDS hCam = 1;
    uint pixelClock = 216;
    double framerate = 1;
    double exposure = 999; // 0.03;
    uint formatID = 36;
    char *pMem = NULL;
    int memID = 0;
    int camera_ready = 0;

    // INT colorMode = IS_CM_MONO8; //CHANGED:MONO8
    // INT colorMode = IS_CM_SENSOR_RAW8;
    int colorMode = IS_CM_SENSOR_RAW16;
    int bitdepth = 16;

    std::string StarName = "None";
    std::string imagePath = "/home/lstoperator/CDM/images/";
    std::string fitsPath = "/home/lstoperator/CDM/fits/";
    std::string remoteImagePathPrefix = "/fefs/home/lapp/CDM_Images/";
    // std::string remoteImagePath = "None";

    double zenith = 0;
    double azimuth = 0;
    double RA = 0;
    double DEC = 0;
    double offset_azimuth = 0;
    double offset_zenith = 0;

    // declare a new attribut
    DataAccessClientOPCUA *m_clientOpcUaRef = NULL;

    std::string getImage();
    int setPixelClock(uint setPixelClock);
    int setFramerate(double setFramerate);
    int setExposure(double setExposure);
    int setStarName(std::string starname);

    int setOffsetAzimuth(double offset_azimuth);
    int setOffsetZenith(double offset_zenith);

    // declare a  new method
    int connectOpcUa(std::string url);
};
#endif //  CDM_H_
