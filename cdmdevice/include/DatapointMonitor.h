#ifndef DATAPOINTMONITOR_H_
#define DATAPOINTMONITOR_H_

#include <MOS_callbackInterface.h>

#include "Helper.h"
class CDM;

#include "Config.h"
using namespace std;

#define DATABROKER_CONFIGURATION_NAME "PLC_DATABROKER.xml"

extern Helper helper;

class DatapointMonitor : public MOS_CallbackInterface {
public:
    DatapointMonitor(CDM *caller);
    void  dataChange(std::vector<std::string> listElements, std::vector<std::string> listValues, std::vector<int> listQuality, std::vector<std::string> listTime);

    void infoDebug(){};
    void abort(){};

    std::vector<std::string> getElements();
    std::vector<int> getNameSpaces();

private:

    CDM *caller;
    
    //Read the configuration and set up DP names
    Config *DB_config = new Config(DATABROKER_CONFIGURATION_NAME,"");
    const std::string leds_var_name =    helper.searchDatapoint("LEDPositions",DB_config);
    const std::string shutter_var_name =  helper.searchDatapoint("Shutter",DB_config); 
    const std::string sis_var_name =      helper.searchDatapoint("SIS",DB_config); 

    const std::string oarl_var_name =             helper.searchDatapoint("OARLRelay_Status",DB_config); 
    const std::string aux_DMEastBottom_var_name =  helper.searchDatapoint("DM_East_Bottom",DB_config); 
    const std::string aux_DMEastTop_var_name =     helper.searchDatapoint("DM_East_Top",DB_config); 
    const std::string aux_DMWestBottom_var_name =  helper.searchDatapoint("DM_West_Bottom",DB_config); 
    const std::string aux_DMWestTop_var_name =     helper.searchDatapoint("DM_West_Bottom",DB_config); 

    const std::string drive_ra_target_var_name =   helper.searchDatapoint("RA_Target",DB_config); 
    const std::string drive_dec_target_var_name =  helper.searchDatapoint("Dec_Target",DB_config); 

    const std::string az_offset_var_name = helper.searchDatapoint("Azimuth_Offset",DB_config); 
    const std::string zd_offset_var_name = helper.searchDatapoint("ZenithAngle_Offset",DB_config); 
    const std::string source_var_name =    helper.searchDatapoint("SourceName",DB_config); 
    const std::string ra_tel_var_name =    helper.searchDatapoint("RA_Telescope",DB_config); 
    const std::string dec_tel_var_name =   helper.searchDatapoint("Dec_Telescope",DB_config); 
    const std::string az_var_name =        helper.searchDatapoint("azimuth_position",DB_config); 
    const std::string zd_var_name =        helper.searchDatapoint("zenithangle_position",DB_config); 

    const std::string drive_inmotion_var_name =    helper.searchDatapoint("Status_In_Motion",DB_config); 
    const std::string drive_inparkinpos_var_name = helper.searchDatapoint("Status_In_Parking_Position",DB_config); 
    const std::string drive_parked_var_name =      helper.searchDatapoint("Status_Parked",DB_config); 
    const std::string drive_tracking_var_name =    helper.searchDatapoint("Status_Tracking_In_Progress",DB_config); 



    void CheckRaUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckDecUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckAzUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckZdUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckAzOffsetUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckZdOffsetUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckSourceUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckOARLUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckLEDsUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckShutterUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckSISUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);

    void CheckDriveInMotionUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckDriveInparkingPosUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckDriveParkedUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckDriveTrackingUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckDriveRaTargetUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckDriveDecTargetUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);


    void CheckAuxDMEastBottomUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckAuxDMEastTopUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckAuxDMWestBottomUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckAuxDMWestTopUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);

    Config *cdm_config;
    
};

#endif //DATAPOINTMONITOR_H_
