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
#include <ctime>
#include <iomanip> // Used for setprecision
#include <cmath>
#include <unistd.h> // TODO: why is this used?

#include "pluginsBase.h"	//TODO: move this to .h file? Was in .cpp before
#include "lappThread.h" // needed for MOS

#include <ueye.h> // IDS camera


class DataAccessClientOPCUA;
class PluginsBase;
class SetDatapointThread : public LAPPThread
{
public:
    // TODO: refactor SetDatapointThread as a template?
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

    // new virtual methods who replace the setAnay getAny methods  with the version 4.0 of MOS
    int get(std::string chaine, int commandStringAck, std::vector<boost::any> *tabValue);
    int set(std::string chaine, int commandStringAck, std::vector<boost::any> tabValue);

    int Connect();

    // Camera stuff
    HIDS hCam = 0;
    SENSORINFO sInfo;
    HWND hWndDisplay = NULL; //DIB mode will be used for display
    char* pcImageMemory;
    int DisplayWidth, DisplayHeight;
    // Need to find out the memory size of the pixel and the colour mode
    int nColorMode = IS_CM_MONO8; //IS_CM_SENSOR_RAW16;
    int nBitsPerPixel = 8; //16;

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
