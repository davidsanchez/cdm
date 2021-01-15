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
#include <unistd.h> // TODO: why is this used? For usleep for example.

#include "pluginsBase.h"	//TODO: move this to .h file? Was in .cpp before
#include "lappThread.h" // needed for MOS
#include "AsynchronousThread.h"
#include "DatapointMonitor.h"

#include <boost/algorithm/string.hpp>
//#include <boost/bimap.hpp>
//#include <boost/assign.hpp>

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

    int AddComment(std::string comment);


    int UpdateTestValues(float newvalue);
    
    int UpdateRAValue(double newvalue);
    int UpdateDecValue(double newvalue);
    int UpdateAzValue(double newvalue);
    int UpdateZdValue(double newvalue);
    int UpdateAzOffsetValue(double newvalue);
    int UpdateZdOffsetValue(double newvalue);
    int UpdateSourceValue(std::string newvalue);
    int UpdateOARLValue(bool newvalue);
    int UpdateLEDsValue(bool newvalue);
    int UpdateLED01Value(int newvalue);
    int UpdateShutterValue(int newvalue);
    int UpdateSISValue(int newvalue);

    int UpdateDriveInMotionValue(bool newvalue);
    int UpdateDriveInParkingPosValue(bool newvalue);
    int UpdateDriveParkedValue(bool newvalue);
    int UpdateDriveTrackingValue(bool newvalue);















private:

    std::string element_opcua_cdm_image = "MOS_Server.CDM.Image.Image_v";
    AsynchronousThread *m_Thread;
    AsynchronousThread *m_ThreadMeteo;


};
#endif //  CDM_H_
