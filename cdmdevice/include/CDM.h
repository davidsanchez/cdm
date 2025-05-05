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

#include <map>

//#include <boost/bimap.hpp>
//#include <boost/assign.hpp>

//#include "Controller.h"

//class CDM : public Controller
class CDM : public PluginsBase
{
public:
    int init(const std::string& chaine);
    int close();
    int cmd(const std::string& chaine, int commandStringAck, std::string& result);

    // new virtual methods appears with the version 3.0 of MOS
    //void searchDatapoint (string element, string &nodeIdL1, int &namespaceL1);
    std::string  searchDatapoint (string element);

    int afterStart();
    int cmdAsynch(const std::string& command, int commandStringAck, const std::string& datapointName, int nameSpace,
                  std::string& result);

    // new virtual methods who replace the setAnay getAny methods  with the version 4.0 of MOS
    int get(const std::string& chaine, int commandStringAck, std::vector<boost::any>& tabValue);
    int set(const std::string& chaine, int commandStringAck, std::vector<boost::any>& tabValue);

    int AddComment(std::string comment);

    int UpdateRaValue(double newvalue);
    int UpdateDecValue(double newvalue);
    int UpdateAzValue(double newvalue);
    int UpdateZdValue(double newvalue);
    int UpdateAzOffsetValue(double newvalue);
    int UpdateZdOffsetValue(double newvalue);
    int UpdateSourceValue(std::string newvalue);
    int UpdateOARLValue(bool newvalue);
    int UpdateLEDsValue(bool newvalue);
    //int UpdateLED01Value(int newvalue);
    int UpdateShutterValue(int newvalue);
    int UpdateSISValue(int newvalue);

    int UpdateDriveInMotionValue(bool newvalue);
    int UpdateDriveInParkingPosValue(bool newvalue);
    int UpdateDriveParkedValue(bool newvalue);
    int UpdateDriveTrackingValue(bool newvalue);
    int UpdateDriveRaTargetValue(double newvalue);
    int UpdateDriveDecTargetValue(double newvalue);

    int UpdateAuxDMEastBottomValue(bool newvalue);
    int UpdateAuxDMEastTopValue(bool newvalue);
    int UpdateAuxDMWestBottomValue(bool newvalue);
    int UpdateAuxDMWestTopValue(bool newvalue);

    DatapointMonitor *dp_monitor_DataBroker = NULL;

    int subscribe_DataBroker();
    int connection_result_DataBroker;


private:

    std::string element_opcua_cdm_image = "MOS_Server.CDM.Image.Image_v";
    AsynchronousThread *m_Thread;
    AsynchronousThread *m_ThreadMeteo;
    AsynchronousThread *m_ThreadLogRestart;

    map<std::string,std::string> m_config;
    Config *cdm_config;
    Config *DB_config;


};
#endif //  CDM_H_
